/**
 * @file telnet-socket-impl.cpp
 * @version 0.5.0
 * @release_date October 17, 2025
 *
 * @brief Implementation of Telnet socket constructor, `InputProcessor`, and utility functions.
 * @remark Contains implementations for `socket` constructor, `InputProcessor`, `sync_await`, and `escape_telnet_output` overloads.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see `:socket` for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, `:types` for `TelnetCommand`, `:options` for `option::id_num`, `:errors` for error codes, `:protocol_fsm` for `ProtocolFSM`
 */
module; // Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

// Module partition implementation unit
module telnet:socket;

import std; // For std::promise, std::future, std::jthread, std::exception_ptr, std::make_tuple

import :types;        ///< @see telnet-types.cppm for `byte_t` and `TelnetCommand`
import :errors;       ///< @see telnet-errors.cppm for `telnet::error` codes
import :options;      ///< @see telnet-options.cppm for `option` and `option::id_num`
import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm for `ProtocolFSM`

import :socket;       ///< @see telnet-socket.cppm for socket partition interface

namespace telnet {
    /**
     * @internal
     * Moves the provided `socket` into `next_layer_`, default-constructs `fsm_`, and enables SO_OOBINLINE.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    socket<NLS, PC>::socket(next_layer_type&& next_layer_socket)
        : next_layer_(std::move(next_layer_socket)), fsm_()
    {
        std::error_code ec;
        next_layer_.lowest_layer().set_option(asio::socket_base::out_of_band_inline(true), ec);
        if (ec) {
            ProtocolConfig::log_error(ec, "Failed to enable out_of_band_inline on socket: {}", ec.message());
        }
    } // socket::socket(next_layer_type&&)

    /**
     * @internal
     * Uses a temporary `asio::io_context` and `std::jthread` to run the awaitable via `asio::co_spawn`.
     * @remark Captures the result or exception using `std::promise` and `std::future`.
     * @remark Deduces `ResultType` with `asio::awaitable_traits` to handle `void` and non-`void` return types.
     * @remark Uses a lambda to set the promise’s value or exception, handling multiple return values via `std::make_tuple`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename Awaitable>
    auto socket<NLS, PC>::sync_await(Awaitable&& a) {
        using ResultType = typename asio::awaitable_traits<std::decay_t<Awaitable>>::return_type;
        asio::io_context temp_ctx;
        std::promise<ResultType> promise;
        std::future<ResultType> future = promise.get_future();

        asio::co_spawn(
            temp_ctx,
            std::forward<Awaitable>(a),
            [&promise](std::exception_ptr e, auto... result) {
                if (e) {
                    promise.set_exception(e);
                } else {
                    if constexpr (std::is_void_v<ResultType>) {
                        promise.set_value();
                    } else {
                        promise.set_value(std::get<0>(std::make_tuple(result...)));
                    }
                }
            } // lambda
        ); // co_spawn

        std::jthread runner_thread([&temp_ctx]{ temp_ctx.run(); });
        return future.get();
    } // socket::sync_await(Awaitable&&)

    /**
     * @internal
     * Iterates over `data` using `asio::buffers_begin` and `asio::buffers_end`.
     * @remark Appends each byte to `escaped_data`, duplicating 0xFF (IAC) bytes plus transforming LF to CR LF and CR to CR NUL when not transmitting in BINARY mode.
     * @remark Catches `std::bad_alloc` to clear `escaped_data` and return `std::errc::not_enough_memory`.
     * @remark Catches other exceptions to clear `escaped_data` and return `telnet::error::internal_error`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
    std::tuple<std::error_code, std::vector<byte_t>&> socket<NLS, PC>::escape_telnet_output(std::vector<byte_t>& escaped_data, const ConstBufferSequence& data) const noexcept {
        try {
            for (auto iter = asio::buffers_begin(data), end = asio::buffers_end(data); iter != end; ++iter) {
                if ((*iter == static_cast<byte_t>('\n')) && !fsm_.enabled(option::id_num::BINARY, NegotiationDirection::LOCAL)) {
                    escaped_data.push_back('\r'); // prepend CR before LF (LF -> CR LF)
                }
                escaped_data.push_back(*iter);
                if (*iter == std::to_underlying(TelnetCommand::IAC)) {
                    escaped_data.push_back(*iter); // double IAC (IAC -> IAC IAC)
                } else if ((*iter == static_cast<byte_t>('\r')) && !fsm_.enabled(option::id_num::BINARY, NegotiationDirection::LOCAL)) {
                    escaped_data.push_back('\0'); // append NUL after CR (CR -> CR NUL)
                }
            }
            return {std::error_code(), escaped_data};
        } catch (const std::bad_alloc&) {
            escaped_data.clear();
            return {std::make_error_code(std::errc::not_enough_memory), escaped_data};
        } catch (...) {
            escaped_data.clear();
            return {std::make_error_code(error::internal_error), escaped_data};
        }
    } // socket::escape_telnet_output(std::vector<byte_t>&, const ConstBufferSequence&) const noexcept

    /**
     * @internal
     * Creates a new `std::vector<byte_t>` with 10% extra capacity based on `asio::buffer_size(data)`.
     * @remark Delegates to the overload with a provided vector.
     * @remark Catches `std::bad_alloc` to return `std::errc::not_enough_memory` with an empty vector.
     * @remark Catches other exceptions to return `telnet::error::internal_error` with an empty vector.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
    std::tuple<std::error_code, std::vector<byte_t>> socket<NLS, PC>::escape_telnet_output(const ConstBufferSequence& data) const noexcept {
        std::vector<byte_t> escaped_data;
        try {
            escaped_data.reserve(asio::buffer_size(data) * 1.1);
            return this->escape_telnet_output(escaped_data, data);
        } catch (const std::bad_alloc&) {
            return {std::make_error_code(std::errc::not_enough_memory), std::vector<byte_t>()};
        } catch (...) {
            return {std::make_error_code(error::internal_error), std::vector<byte_t>()};
        }
    } // socket::escape_telnet_output(const ConstBufferSequence&) const noexcept

    /**
     * @internal
     * Launches an asynchronous wait (via 0-byte async_receive) for notification of urgent (i.e. "out-of-band") data.
     * @remark Only launches the async_receive if another one is not already in progress and there is not already urgent data in the byte stream.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig, PC>
    void socket<NLS, PC>::launch_wait_for_urgent_data() {
        if (!context_.waiting_for_urgent_data.exchange(true, std::memory_order_relaxed)
            && !context_.urgent_data_state) {
            this->lowest_layer().async_receive(
                asio::mutable_buffer(nullptr, 0),
                asio::socket_base::message_out_of_band,
                asio::bind_executor(
                    this->get_executor(),
                    [this](std::error_code ec, std::size_t) {
                        this->context_.waiting_for_urgent_data.store(false, std::memory_order_relaxed);
                        if (!ec) {
                            this->context_.urgent_data_state.saw_urgent();
                        } else {
                            fsm_type::ProtocolConfig::log_error(ec, "OOB wait failed: {}", ec.message());
                            if (!this->context_.deferred_transport_error) {
                                this->context_.deferred_transport_error = ec;
                            } // If there is already a transport error deferred, ignore this one as it’s likely redundant.
                        }
                    } // [this](std::error_code, std::size_t)
                )
            );
        }
    } // socket::launch_urgent_wait()

    /**
     * @internal
     * Use `compare_exchange_strong` to update `state_`.
     * If `state_` was `NO_URGENT_DATA`, it becomes `HAS_URGENT_DATA`.
     * If `state_` was `UNEXPECTED_DATA_MARK`, it resets to `NO_URGENT_DATA`.
     * It can't already be `HAS_URGENT_DATA` without a major bug.
     */    
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    void socket<NLS, PC>::context_type::urgent_data_tracker::saw_urgent() {
        UrgentDataState expected;
        UrgentDataState desired;
        bool success = false;
    
        do {
            // Load the current state
            expected = state_.load(std::memory_order_relaxed);
    
            if (expected == UrgentDataState::NO_URGENT_DATA) {
                // The OOB notification arrived first.
                desired = UrgentDataState::HAS_URGENT_DATA;
            } else if (expected == UrgentDataState::UNEXPECTED_DATA_MARK) {
                // The DM arrived first; this is the delayed notification. Reset.
                ProtocolConfig::log_error(processing_signal::data_mark, "DM already arrived before current TCP urgent notification. Assuming Synch is already complete.");
                desired = UrgentDataState::NO_URGENT_DATA;
            } else {
                // CANT HAPPEN: State is `HAS_URGENT_DATA`. This means another saw_urgent fired without saw_data_mark in between, or a logic error.
                // We cannot transition and must exit.
                ProtocolConfig::log_error(error::internal_error, "Invalid state in saw_urgent: HAS_URGENT_DATA already set; implies launch_wait_for_urgent_data was called while urgent data was already in the byte stream.");
                return;
            }
    
            // Atomically attempt the transition
            success = state_.compare_exchange_strong(
                expected, 
                desired,
                std::memory_order_release, // Ensure subsequent reads see the change
                std::memory_order_relaxed
            );
    
        } while (!success);
    } // socket::context_type::urgent_data_tracker::saw_urgent()

    /**
     * @internal
     * Use `compare_exchange_strong` to update `state_`.
     * If `state_` was `NO_URGENT_DATA`, it becomes `UNEXPECTED_DATA_MARK`.
     * If `state_` was `HAS_URGENT_DATA`, it resets to `NO_URGENT_DATA`.     
     * It can't already be `UNEXPECTED_DATA_MARK` without a major bug.
     */    
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    void socket<NLS, PC>::context_type::urgent_data_tracker::saw_data_mark() {
        UrgentDataState expected;
        UrgentDataState desired;
        bool success = false;
    
        do {
            // Load the current state
            expected = state_.load(std::memory_order_relaxed);
    
            if (expected == UrgentDataState::HAS_URGENT_DATA) {
                // The DM arrived as expected. Reset.
                desired = UrgentDataState::NO_URGENT_DATA;
            } else if (expected == UrgentDataState::NO_URGENT_DATA) {
                // The DM arrived before the OOB notification arrived.
                desired = UrgentDataState::UNEXPECTED_DATA_MARK;
                ProtocolConfig::log_error(processing_signal::data_mark, "DM arrived without/before TCP urgent.");
            } else {
                // State is `UNEXPECTED_DATA_MARK`. This means another `saw_data_mark` fired without `saw_urgent` in between, or a logic error. The peer likely sent 2 data marks in quick succession, but this is safe.
                // We cannot transition and must exit.
                ProtocolConfig::log_error(processing_signal::data_mark, "Subsequent DM received while expecting TCP urgent.");
                return;
            }
    
            // Atomically attempt the transition
            success = state_.compare_exchange_strong(
                expected, 
                desired,
                std::memory_order_release, // Ensure subsequent reads see the change
                std::memory_order_relaxed
            );
    
        } while (!success);
    } // socket::context_type::urgent_data_tracker::saw_data_mark()

    /**
     * @internal
     * Initializes member variables `parent_socket_`, `fsm_`, `buffers_`, and sets `state_` to `INITIALIZING`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
    socket<NLS, PC>::InputProcessor<MutableBufferSequence>::InputProcessor(
        socket& parent_socket, socket::fsm_type& fsm, socket::context_type& context, MutableBufferSequence buffers)
        : parent_socket_(parent_socket),
          fsm_(fsm),
          context_(context),
          buffers_(buffers),
          state_(State::INITIALIZING) {}

    /**
     * @internal
     * Manages state transitions using a switch statement (`INITIALIZING`, `READING`, `PROCESSING`, `DONE`).
     * - In `INITIALIZING`, calls `next_layer_.async_read_some` with `buffers_` unless `context_.input_side_buffer` already has data.
     * - In `READING`, sets up iterators (`user_buf_begin_`, `user_buf_end_`, `write_it_`) and transitions to `PROCESSING`.
     * - In `PROCESSING`, iterates over the buffer, calling `fsm_.process_byte`, handling forward flags (outside of urgent/Synch mode), and processing `ProcessingReturnVariant` responses via `std::visit`:
     *   - `NegotiationResponse`: Calls `async_write_negotiation`.
     *   - `std::string`: Calls `async_write_raw` with shared buffer.
     *   - `SubnegotiationAwaitable`: Spawns coroutine to `co_await` the handler.
     *   - `std::tuple{OptionEnablementAwaitable, std::optional<NegotiationResponse>}` or `std::tuple{OptionDisablementAwaitable, std::optional<NegotiationResponse>}`: Spawns coroutine to `co_await` `async_write_negotiation` (if negotiation exists), then `co_await` the handler, accumulating bytes transferred.
     * - Consumes bytes from the `context_.input_side_buffer` manually before async operations to avoid re-processing.
     * - Completes with `std::distance(user_buf_begin_, write_it_)` bytes when `read_it == buf_end` or `write_it_ == user_buf_end_` or `process_byte` returns an error code. [std::distance should be linear time in the number of buffers in the sequence rather than the number of bytes]
     * @remark Handles EC and EL internally if there is buffered output to edit; otherwise, signals caller.
     * @remark Handles AO by clearing `context_.output_side_buffer` but propagates the signal to the caller for higher-level notification.
     * @note Unhandled `processing_signal`s and other `error_code`s propagate to the caller for higher-level notification. 
     * @remark Uses `[[unlikely]]` for `DONE` state and default case, and `[[fallthrough]]` for `READING` to `PROCESSING`.
     * @remark Checks `DONE` state early to prevent reentrancy.
     * @todo Phase6: Refactor helper methods to simplify main processing loop.
     * @todo Phase6: Refactor overloaded helper methods for std::visit.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence, typename Self>
    void socket<NLS, PC>::InputProcessor<MutableBufferSequence>::operator()(Self& self, std::error_code ec_in, std::size_t bytes_transferred) {
        if (state_ == State::DONE) [[unlikely]] {
            return; // complete has already been called; unsafe to do anything else
        }
        
        switch (state_) {
            case State::INITIALIZING:
                state_ = State::READING;
                if (context_.input_side_buffer.size() == 0) {
                    if (context_.deferred_transport_error) {
                        // Immediately propagate a deferred error without attempting the next read.
                        complete(self, std::exchange(context_.deferred_transport_error, {}), 0);
                        return;
                    }
                    
                    parent_socket_.launch_wait_for_urgent_data();
                    
                    auto read_buffer = context_.input_side_buffer.prepare(READ_BLOCK_SIZE);
                    parent_socket_.next_layer().async_read_some(
                        read_buffer,
                        asio::bind_executor(parent_socket_.get_executor(), std::move(self))
                    );
                    return; // Wait for next_layer async_read_some to complete.
                }
                [[fallthrough]]; // If the buffer already has data, there is no need to wait for a network read.
            case State::READING:
                context_.input_side_buffer.commit(bytes_transferred);
                
                if (context_.input_side_buffer.size() == 0) {
                    // If there is no data to process, we can complete, propagating any read error. 
                    complete(self, ec_in, 0);
                    return;
                }
                
                if (ec_in) {
                    // If we had a deferred error, we skipped the read in State::INITIALIZING.
                    // If we have a read error, defer it, and clear ec_in for State::PROCESSING.
                    std::swap(context_.deferred_transport_error, ec_in);
                }
                
                user_buf_begin_ = asio::buffers_begin(buffers_);
                user_buf_end_   = asio::buffers_end(buffers_);
                write_it_       = user_buf_begin_;
                
                state_ = State::PROCESSING;
                [[fallthrough]];
            case State::PROCESSING:
                if (ec_in) { // This has to be a write error.
                    if (context_.deferred_transport_error) {
                        // We have a new write error on top of a previously deferred error.
                        // Log it and attempt to continue processing the buffered byte stream.
                        decltype(fsm_)::ProtocolConfig::log_error(
                            ec_in,
                            "Error writing Telnet response with error {} previously deferred for reporting after processing the buffered byte stream.",
                            context_.deferred_transport_error
                        );
                    } else {
                        // Defer the write error.
                        context_.deferred_transport_error = ec_in;
                    }
                }
                
                std::error_code result_ec = std::exchange(context_.deferred_processing_signal, {});
                
                std::size_t bytes_consumed = 0;
                std::size_t bytes_to_transfer = 0;

                if (!result_ec) {
                    const auto proc_buffer = context_.input_side_buffer.data();
                    auto buf_begin = asio::buffers_begin(proc_buffer);
                    auto buf_end = asio::buffers_end(proc_buffer);
                    auto read_it = buf_begin;
                    
                    for (; (read_it != buf_end) && (write_it_ != user_buf_end_); ++read_it) {
                        auto [proc_ec, forward, response] = fsm_.process_byte(static_cast<byte_t>(*read_it));
                        
                        // Handle `processing_signal`s that modify the buffer directly.
                        if (proc_ec == processing_signal::carriage_return) {
                            // A previously discarded '\r' byte must be inserted into the user's buffer.
                            *write_it_++ = '\r';
                            proc_ec.clear(); // Further processing is clear to continue.
                        } else if ((proc_ec == processing_signal::erase_character)
                                && (write_it_ != user_buf_begin_)) {
                            // If the user buffer has data, EC backs up the write position by one character.
                            // Otherwise, it propagates to the caller.
                            --write_it_;
                            proc_ec.clear(); // Further processing is clear to continue.
                        } else if ((proc_ec == processing_signal::erase_line)
                                && (write_it_ != user_buf_begin_)) {
                            // If the user buffer has data, EL resets the write position to the beginning of the buffer.
                            // Otherwise, it propagates to the caller.
                            write_it_ = user_buf_begin_;
                            proc_ec.clear(); // Further processing is clear to continue.
                        } else if (proc_ec == processing_signal::abort_output) {
                            // AO clears the output side buffer.
                            context_.output_side_buffer.consume(context_.output_side_buffer.size());
                            
                            // Defer the AO processing signal for application-level notification after the `async_send_synch` completes.
                            context_.deferred_processing_signal = proc_ec;
                            
                            // Consume the bytes currently processed from the buffer INCLUDING this one
                            // MUST call now because we `move(self)` while handling `response` and lose access to `context_.input_side_buffer`
                            bytes_consumed = std::distance(buf_begin, read_it) + 1;
                            context_.input_side_buffer.consume(bytes_consumed);
                            
                            parent_socket_.async_send_synch(std::move(self));
                            return;
                        } else if (proc_ec == processing_signal::data_mark) {
                            context_.urgent_data_state.saw_data_mark();
                            parent_socket_.launch_wait_for_urgent_data();
                        }
                        
                        // Write a `forward`ed byte into the user's buffer.
                        if (forward && !context_.urgent_data_state) {
                            *write_it_++ = *read_it;
                        }
                        if (proc_ec) {
                            // Consume the bytes currently processed from the buffer INCLUDING this one
                            bytes_consumed = std::distance(buf_begin, read_it) + 1;
                            bytes_to_transfer = std::distance(user_buf_begin_, write_it_);
                            result_ec = proc_ec; //Propagate the processing error_code 
                            break; // Complete the operation.
                        }
                        if (response) {
                            // Consume the bytes currently processed from the buffer INCLUDING this one
                            // MUST call now because we `move(self)` while handling `response` and lose access to `context_.input_side_buffer`
                            bytes_consumed = std::distance(buf_begin, read_it) + 1;
                            context_.input_side_buffer.consume(bytes_consumed);
    
                            std::visit([this, self = std::move(self)](auto&& arg) mutable {
                                using T = std::decay_t<decltype(arg)>;
    
                                if constexpr (std::is_same_v<T, typename decltype(fsm_)::NegotiationResponse>) {
                                    // TelOpt negotiation response
                                    parent_socket_.async_write_negotiation(
                                        arg,
                                        std::move(self)
                                    );
                                } else if constexpr (std::is_same_v<T, std::string>) {
                                    // Write a raw response string.
                                    parent_socket_.async_write_raw(
                                        asio::buffer(std::make_shared<std::string>(arg)),
                                        std::move(self)
                                    );
                                } else if constexpr (std::is_same_v<std::decay_t<T>, awaitables::SubnegotiationAwaitable>) {
                                    // Subnegotiation handler
                                    auto handler_awaitable = arg;
                                    asio::co_spawn(parent_socket_.get_executor(), [handler_awaitable = std::move(handler_awaitable)]{
                                        try {
                                            auto [opt, subneg_buffer] = co_await handler_awaitable;
                                            if (!subneg_buffer.empty()) {
                                                co_return co_await parent_socket_.async_write_subnegotiation(opt, subneg_buffer);
                                            } else  {
                                                co_return std::size_t{0};
                                            }
                                        } catch (std::system_error se) {
                                            throw;
                                        } catch (...) {
                                            throw std::system_error(error::internal_error);
                                        }
                                    }, std::move(self));
                                } else if constexpr ((std::is_same_v<T, std::tuple<awaitables::OptionDisablementAwaitable, std::optional<typename decltype(fsm_)::NegotiationResponse>>>) ||
                                                     (std::is_same_v<T, std::tuple<awaitables::OptionDisablementAwaitable, std::optional<typename decltype(fsm_)::NegotiationResponse>>>)) {
                                    // Enablement/Disablement handlers
                                    auto [awaitable, negotiation] = std::move(arg);
                                    asio::co_spawn(parent_socket_.get_executor(), [this, awaitable = std::move(awaitable), negotiation = std::move(negotiation)]() mutable -> asio::awaitable<std::size_t> {
                                        try {
                                            std::size_t bt = 0;
                                            if (negotiation) {
                                                bt += co_await parent_socket_.async_write_negotiation(*negotiation, asio::use_awaitable);
                                            }
                                            co_await awaitable;
                                            co_return bt;
                                        } catch (std::system_error se) {
                                            throw;
                                        } catch (...) {
                                            throw std::system_error(error::internal_error);
                                        }
                                    }, self);
                                }
                            }, *response);
                            return; // Wait for async operation to complete
                        } // if (response)
                    } // for
                    if (!result_ec) {
                        // SUCCESS: We either reached the end of the read data or filled the user's buffer.
                        bytes_consumed = std::distance(buf_begin, read_it);
                        bytes_to_transfer = std::distance(user_buf_begin_, write_it_);
                        
                        // Since there is no current error, swap in any deferred error to report it.
                        std::swap(context_.deferred_transport_error, result_ec);
                    } // if (!result_ec) late check
                } // if (!result_ec) early check
                
                context_.input_side_buffer.consume(bytes_consumed);
                complete(self, result_ec, bytes_to_transfer);
                break;
            case State::DONE: [[fallthrough]];
            default:
                [[unlikely]]
                break; // Unreachable due to early check
        } // switch (state_)
    } // socket::InputProcessor::operator(Self&, std::error_code, std::size_t)

    /**
     * @internal
     * Sets `state_` to `DONE` and calls `self.complete` with `ec` and `bytes_transferred`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MBS, typename Self>
    void socket<NLS, PC>::InputProcessor<MBS>::complete(Self& self, const std::error_code& ec, std::size_t bytes_transferred) {
        state_ = State::DONE;
        self.complete(ec, bytes_transferred);
    } // socket::InputProcessor::complete(Self&, const std::error_code&, std::size_t)
} // namespace telnet