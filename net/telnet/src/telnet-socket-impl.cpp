/**
 * @file telnet-socket-impl.cpp
 * @version 0.4.0
 * @release_date October 3, 2025
 *
 * @brief Implementation of Telnet socket constructor, `InputProcessor`, and utility functions.
 * @remark Contains implementations for `socket` constructor, `InputProcessor`, `sync_await`, and `escape_telnet_output` overloads.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see :socket for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, :types for `TelnetCommand`, :options for `option::id_num`, :errors for error codes, :protocol_fsm for `ProtocolFSM`
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
     * @brief Moves the provided `socket` into `next_layer_` and default-constructs `fsm_`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    socket<NLS, PC>::socket(next_layer_type&& socket)
        : next_layer_(std::move(socket)), fsm_() {}

    /**
     * @internal
     * @brief Uses a temporary `asio::io_context` and `std::jthread` to run the awaitable via `asio::co_spawn`.
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
     * @brief Iterates over `data` using `asio::buffers_begin` and `asio::buffers_end`.
     * @remark Appends each byte to `escaped_data`, duplicating 0xFF (IAC) bytes.
     * @remark Catches `std::bad_alloc` to clear `escaped_data` and return `std::errc::not_enough_memory`.
     * @remark Catches other exceptions to clear `escaped_data` and return `telnet::error::internal_error`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
    std::tuple<std::error_code, std::vector<byte_t>&> socket<NLS, PC>::escape_telnet_output(std::vector<byte_t>& escaped_data, const ConstBufferSequence& data) const noexcept {
        try {
            for (auto iter = asio::buffers_begin(data), end = asio::buffers_end(data); iter != end; ++iter) {
                escaped_data.push_back(*iter);
                if (*iter == std::to_underlying(TelnetCommand::IAC)) {
                    escaped_data.push_back(*iter);
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
     * @brief Creates a new `std::vector<byte_t>` with 10% extra capacity based on `asio::buffer_size(data)`.
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
     * @brief Initializes member variables `parent_socket_`, `fsm_`, `buffers_`, and sets `state_` to `INITIALIZING`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
    socket<NLS, PC>::InputProcessor<MutableBufferSequence>::InputProcessor(
        socket& parent_socket, socket::fsm_type& fsm, MutableBufferSequence buffers)
        : parent_socket_(parent_socket),
          fsm_(fsm),
          buffers_(buffers),
          state_(State::INITIALIZING) {}

    /**
     * @internal
     * @brief Manages state transitions using a switch statement (`INITIALIZING`, `READING`, `PROCESSING`, `DONE`).
     * @remark In `INITIALIZING`, calls `next_layer_.async_read_some` with `buffers_`.
     * @remark In `READING`, sets up iterators (`buf_begin_`, `buf_end_`, `write_it_`, `read_it_`) and transitions to `PROCESSING`.
     * @remark In `PROCESSING`, iterates over the buffer, calling `fsm_.process_byte`, handling forward flags, and processing `ProcessingReturnVariant` responses via `std::visit` for negotiation, raw writes, command handlers, or subnegotiation handlers.
     * @remark Increments `read_it_` manually before async operations to avoid re-processing.
     * @remark Completes with `std::distance(buf_begin_, write_it_)` bytes when `read_it_ == buf_end_`.
     * @remark Uses `[[unlikely]]` for `DONE` state and default case, and `[[fallthrough]]` for `READING` to `PROCESSING`.
     * @remark Checks `DONE` state early to prevent reentrancy.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence, typename Self>
    void socket<NLS, PC>::InputProcessor<MutableBufferSequence>::operator()(Self& self, std::error_code ec, std::size_t bytes_transferred) {
        if (state_ == State::DONE) [[unlikely]] {
            return; // complete has already been called; unsafe to do anything else
        } else if (ec) {
            complete(self, ec, 0);
            return;
        }
        switch (state_) {
            case State::INITIALIZING:
                state_ = State::READING;
                parent_socket_.next_layer().async_read_some(
                    buffers_,
                    asio::bind_executor(parent_socket_.get_executor(), std::move(self))
                );
                break;
            case State::READING:
                buf_begin_ = asio::buffers_begin(buffers_);
                buf_end_ = buf_begin_ + bytes_transferred;
                write_it_ = buf_begin_;
                read_it_ = buf_begin_;
                state_ = State::PROCESSING;
                [[fallthrough]];
            case State::PROCESSING:
                for (; read_it_ != buf_end_; ++read_it_) {
                    auto [proc_ec, forward, response] = fsm_.process_byte(*read_it_);
                    if (proc_ec) {
                        complete(self, proc_ec, 0);
                        break;
                    }
                    if (forward) {
                        if (write_it_ >= buf_end_) {
                            // Invariant Violation
                            complete(self, std::make_error_code(error::internal_error), 0);
                            break;
                        }
                        *write_it++ = *read_it_;
                    }
                    if (response) {
                        // Pausing processing for async_write_negotiation breaks the loop and prevents the for-loop-based increment.
                        // Do it manually before handing over control to avoid re-processing.
                        ++read_it_;

                        std::visit([self = std::move(self)](auto&& arg){
                            using T = std::decay_t<decltype(arg)>;

                            if constexpr (std::is_same_v<T, std::tuple<TelnetCommand, option::id_num>>) {
                                // TelOpt negotiation response
                                auto [cmd, opt] = arg;
                                parent_socket_.async_write_negotiation(
                                    cmd,
                                    static_cast<option::id_num>(opt),
                                    std::move(self)
                                );
                            } else if constexpr (std::is_same_v<T, std::string>) {
                                // Write a raw response string.
                                parent_socket_.async_write_raw(
                                    asio::buffer(arg),
                                    std::move(self)
                                );
                            } else if constexpr (std::is_same_v<T, std::tuple<TelnetCommand, decltype(parent_socket_)::fsm_type::TelnetCommandHandler>>) {
                                // Command handler
                                auto [cmd, handler] = arg;
                                asio::co_spawn(parent_socket_.get_executor(), [cmd, &handler]{
                                    try {
                                        co_await handler(cmd);
                                        co_return std::size_t{0};
                                    } catch (std::system_error se) {
                                        throw;
                                    } catch (...) {
                                        throw std::system_error(error::internal_error);
                                    }
                                }, std::move(self));
                            } else if constexpr (std::is_same_v<T, decltype(parent_socket_)::fsm_type::SubnegotiationAwaitable>) {
                                // Subnegotiation handler
                                auto handler_awaitable = arg;
                                asio::co_spawn(parent_socket_.get_executor(), [&handler_awaitable]{
                                    try {
                                        co_await handler_awaitable;
                                        co_return std::size_t{0};
                                    } catch (std::system_error se) {
                                        throw;
                                    } catch (...) {
                                        throw std::system_error(error::internal_error);
                                    }
                                }, std::move(self));
                            }
                        }, *response);
                        break; // Wait for async operation to complete
                    }
                }
                if (read_it_ == buf_end_) {
                    complete(self, std::error_code(), std::distance(buf_begin_, write_it_));
                }
                break;
            case State::DONE: [[fallthrough]]
            default:
                [[unlikely]]
                break; // Unreachable due to early check
        }
    } // socket::InputProcessor::operator(Self&, std::error_code, std::size_t)

    /**
     * @internal
     * @brief Sets `state_` to `DONE` and calls `self.complete` with `ec` and `bytes_transferred`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
    template<typename Self>
    void socket<NLS, PC>::InputProcessor<MutableBufferSequence>::complete(Self& self, const std::error_code& ec, std::size_t bytes_transferred) {
        state_ = State::DONE;
        self.complete(ec, bytes_transferred);
    } // socket::InputProcessor::complete(Self&, const std::error_code&, std::size_t)
} // namespace telnet