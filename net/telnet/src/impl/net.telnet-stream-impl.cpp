// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
/**
 * @file net.telnet-stream-impl.cpp
 * @version 0.5.7
 * @date October 30, 2025
 *
 * @copyright © 2025-2026 Jeremy Murphy and any Contributors
 * @par License: @parblock
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. @endparblock
 *
 * @brief Implementation of Telnet stream constructor, `input_processor`, and utility functions.
 * @remark Contains implementations for `stream` constructor, `input_processor`, `sync_await`, and `escape_telnet_output` overloads.
 *
 * @see "net.telnet-stream.cppm" for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, `:types` for `telnet::command`, `:options` for `option::id_num`, `:errors` for error codes, `:protocol_fsm` for `ProtocolFSM`
 */

module; //Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio.hpp>

//Module implementation unit
module net.telnet;

import std; //NOLINT For std::promise, std::future, std::jthread, std::exception_ptr, std::make_tuple

import :types;        ///< @see "net.telnet-types.cppm" for `byte_t` and `telnet::command`
import :errors;       ///< @see "net.telnet-errors.cppm" for `telnet::error` and `telnet::processing_signal` codes
import :concepts;     ///< @see "net.telnet-concepts.cppm" for `telnet::concepts::LayerableSocketStream`
import :options;      ///< @see "net.telnet-options.cppm" for `option` and `option::id_num`
import :protocol_fsm; ///< @see "net.telnet-protocol_fsm.cppm" for `ProtocolFSM`
import :awaitables;   ///< @see "net.telnet-awaitables.cppm" for awaitable types

import :stream; ///< @see "net.telnet-stream.cppm" for the partition being implemented.

//namespace asio = boost::asio;

namespace net::telnet {
    /**
     * @internal
     * Moves the provided `next_layer_stream` into `next_layer_`, default-constructs `fsm_`, and enables SO_OOBINLINE.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    stream<NLS, PC>::stream(next_layer_type&& next_layer_stream) : next_layer_(std::move(next_layer_stream)), fsm_()
    {
        std::error_code ec;
        next_layer_.lowest_layer().set_option(lowest_layer_type::out_of_band_inline(true), ec);
        if (ec) {
            PC::log_error(ec, "Failed to enable out_of_band_inline on socket: {}", ec.message());
        }
    } //stream::stream(next_layer_type&&)

    /**
     * @internal
     * Uses a temporary `asio::io_context` and `std::jthread` to run the awaitable via `asio::co_spawn`.
     * @remark Captures the result or exception using `std::promise` and `std::future`.
     * @remark Deduces `result_type` with `asio::awaitable_traits` to handle `void` and non-`void` return types.
     * @remark Uses a lambda to set the promise’s value or exception, handling multiple return values via `std::make_tuple`.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<typename Awaitable>
    auto stream<NLS, PC>::sync_await(Awaitable&& awaitable)
    {
        using result_type = typename Awaitable::value_type;
        asio::io_context temp_ctx;
        std::promise<result_type> promise;
        std::future<result_type> future = promise.get_future();

        asio::co_spawn(
            temp_ctx, std::forward<Awaitable>(awaitable), [&promise](std::exception_ptr excpt, auto... result) {
                if (excpt) {
                    promise.set_exception(excpt);
                } else {
                    if constexpr (std::is_void_v<result_type>) {
                        promise.set_value();
                    } else {
                        promise.set_value(std::get<0>(std::make_tuple(result...)));
                    }
                }
            } //lambda
        ); //co_spawn

        std::jthread runner_thread([&temp_ctx] { temp_ctx.run(); });
        return future.get();
    } //stream::sync_await(Awaitable&&)

    /**
     * @internal
     * Iterates over `data` using `asio::buffers_begin` and `asio::buffers_end`.
     * @remark Appends each byte to `escaped_data`, duplicating 0xFF (IAC) bytes plus transforming LF to CR LF and CR to CR NUL when not transmitting in BINARY mode.
     * @remark Catches `std::bad_alloc` to clear `escaped_data` and return `std::errc::not_enough_memory`.
     * @remark Catches other exceptions to clear `escaped_data` and return `telnet::error::internal_error`.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<ConstBufferSequence CBufSeq>
    std::tuple<std::error_code, std::vector<byte_t>&>
        stream<NLS, PC>::escape_telnet_output(std::vector<byte_t>& escaped_data, const CBufSeq& data) const noexcept
    {
        try {
            for (auto iter = asio::buffers_begin(data), end = asio::buffers_end(data); iter != end; ++iter) {
                if ((*iter == static_cast<byte_t>('\n'))
                    && !fsm_.enabled(option::id_num::binary, negotiation_direction::local)) {
                    escaped_data.push_back('\r'); //prepend CR before LF (LF -> CR LF)
                }
                escaped_data.push_back(*iter);
                if (*iter == std::to_underlying(telnet::command::iac)) {
                    escaped_data.push_back(*iter); //double IAC (IAC -> IAC IAC)
                } else if ((*iter == static_cast<byte_t>('\r'))
                           && !fsm_.enabled(option::id_num::binary, negotiation_direction::local)) {
                    escaped_data.push_back('\0'); //append NUL after CR (CR -> CR NUL)
                }
            }
            return {std::error_code(), escaped_data};
        } catch (const std::bad_alloc&) {
            escaped_data.clear();
            return {make_error_code(std::errc::not_enough_memory), escaped_data};
        } catch (...) {
            escaped_data.clear();
            return {make_error_code(error::internal_error), escaped_data};
        }
    } //stream::escape_telnet_output(std::vector<byte_t>&, const CBufSeq&) const noexcept

    /**
     * @internal
     * Creates a new `std::vector<byte_t>` with 10% extra capacity based on `asio::buffer_size(data)`.
     * @remark Delegates to the overload with a provided vector.
     * @remark Catches `std::bad_alloc` to return `std::errc::not_enough_memory` with an empty vector.
     * @remark Catches other exceptions to return `telnet::error::internal_error` with an empty vector.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<ConstBufferSequence CBufSeq>
    std::tuple<std::error_code, std::vector<byte_t>> stream<NLS, PC>::escape_telnet_output(const CBufSeq& data) const noexcept
    {
        std::vector<byte_t> escaped_data;
        try {
            constexpr double escaping_cushion_factor = 1.1;
            escaped_data.reserve(asio::buffer_size(data) * escaping_cushion_factor);
            return this->escape_telnet_output(escaped_data, data);
        } catch (const std::bad_alloc&) {
            return {make_error_code(std::errc::not_enough_memory), std::vector<byte_t>()};
        } catch (...) {
            return {make_error_code(error::internal_error), std::vector<byte_t>()};
        }
    } //stream::escape_telnet_output(const CBufSeq&) const noexcept

    /**
     * @internal
     * Launches an asynchronous wait (via 0-byte async_receive) for notification of urgent (i.e. "out-of-band") data.
     * @remark Only launches the async_receive if another one is not already in progress and there is not already urgent data in the byte stream.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    void stream<NLS, PC>::launch_wait_for_urgent_data()
    {
        if (!context_.waiting_for_urgent_data.exchange(true, std::memory_order_relaxed) && !context_.urgent_data_state) {
            this->lowest_layer().async_receive(
                asio::mutable_buffer(nullptr, 0),
                asio::socket_base::message_out_of_band,
                asio::bind_executor(
                    this->get_executor(), [this](std::error_code ec, std::size_t) {
                        this->context_.waiting_for_urgent_data.store(false, std::memory_order_relaxed);
                        if (!ec) {
                            this->context_.urgent_data_state.saw_urgent();
                        } else {
                            fsm_type::protocol_config_type::log_error(ec, "OOB wait failed: {}", ec.message());
                            if (!this->context_.deferred_transport_error) {
                                this->context_.deferred_transport_error = ec;
                            } //If there is already a transport error deferred, ignore this one as it’s likely redundant.
                        }
                    } //[this](std::error_code, std::size_t)
                )
            );
        }
    } //stream::launch_urgent_wait()

    /**
     * @internal
     * Use `compare_exchange_strong` to update `state_`.
     * If `state_` was `no_urgent_data`, it becomes `has_urgent_data`.
     * If `state_` was `unexpected_data_mark`, it resets to `no_urgent_data`.
     * It can't already be `has_urgent_data` without a major bug.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    void stream<NLS, PC>::context_type::urgent_data_tracker::saw_urgent()
    {
        urgent_data_state expected_state{};
        urgent_data_state desired_state{};
        bool success = false;

        do {
            //Load the current state
            expected_state = state_.load(std::memory_order_relaxed);

            if (expected_state == urgent_data_state::no_urgent_data) {
                //The OOB notification arrived first.
                desired_state = urgent_data_state::has_urgent_data;
            } else if (expected_state == urgent_data_state::unexpected_data_mark) {
                //The DM arrived first; this is the delayed notification. Reset.
                protocol_config_type::log_error(
                    processing_signal::data_mark,
                    "DM already arrived before current TCP urgent notification. Assuming Synch is already complete."
                );
                desired_state = urgent_data_state::no_urgent_data;
            } else {
                //CANT HAPPEN: state is `has_urgent_data`. This means another saw_urgent fired without saw_data_mark in between, or a logic error.
                //We cannot transition and must exit.
                protocol_config_type::log_error(
                    error::internal_error,
                    "Invalid state in saw_urgent: has_urgent_data already set; implies launch_wait_for_urgent_data was " "called while urgent data was already in the byte stream."
                );
                return;
            }

            //Atomically attempt the transition
            success = state_.compare_exchange_strong(
                expected_state,
                desired_state,
                std::memory_order_release, //Ensure subsequent reads see the change
                std::memory_order_relaxed
            );

        } while (!success);
    } //stream::context_type::urgent_data_tracker::saw_urgent()

    /**
     * @internal
     * Use `compare_exchange_strong` to update `state_`.
     * If `state_` was `no_urgent_data`, it becomes `unexpected_data_mark`.
     * If `state_` was `has_urgent_data`, it resets to `no_urgent_data`.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    void stream<NLS, PC>::context_type::urgent_data_tracker::saw_data_mark()
    {
        urgent_data_state expected_state{};
        urgent_data_state desired_state{};
        bool success = false;

        do {
            //Load the current state
            expected_state = state_.load(std::memory_order_relaxed);

            if (expected_state == urgent_data_state::has_urgent_data) {
                //The DM arrived as expected. Reset.
                desired_state = urgent_data_state::no_urgent_data;
            } else if (expected_state == urgent_data_state::no_urgent_data) {
                //The DM arrived before the OOB notification arrived.
                desired_state = urgent_data_state::unexpected_data_mark;
                protocol_config_type::log_error(processing_signal::data_mark, "DM arrived without/before TCP urgent.");
            } else {
                //State is `unexpected_data_mark`. This means another `saw_data_mark` fired without `saw_urgent` in between, or a logic error. The peer likely sent 2 data marks in quick succession, but this is safe.
                //We cannot transition and must exit.
                protocol_config_type::log_error(
                    processing_signal::data_mark, "Subsequent DM received while expecting TCP urgent."
                );
                return;
            }

            //Atomically attempt the transition
            success = state_.compare_exchange_strong(
                expected_state,
                desired_state,
                std::memory_order_release, //Ensure subsequent reads see the change
                std::memory_order_relaxed
            );

        } while (!success);
    } //stream::context_type::urgent_data_tracker::saw_data_mark()

    /**
     * @internal
     * Initializes member variables `parent_stream_`, `fsm_`, `buffers_`, and sets `state_` to `initializing`.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    stream<NLS, PC>::input_processor<MBS>::input_processor(
        stream& parent_stream,
        stream::fsm_type& fsm,
        stream::context_type& context,
        MBS buffers
    )
        : parent_stream_(parent_stream), fsm_(fsm), context_(context), buffers_(buffers), state_(state::initializing)
    {}

    /**
     * @internal
     * Manages state transitions using a switch statement (`initializing`, `reading`, `processing`, `done`).
     * Dispatches to "handle_processor_state_*" helpers for each valid state.
     * @remark Uses `[[unlikely]]` for `done` state and default case, and `[[fallthrough]]` for `reading` to `processing`.
     * @remark Checks `done` state early to prevent reentrancy.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    template<typename Self>
    void stream<NLS, PC>::input_processor<MBS>::operator()(Self& self, std::error_code ec_in, std::size_t bytes_transferred)
    {
        if (state_ == state::done) [[unlikely]] {
            return; //complete has already been called; unsafe to do anything else
        }

        switch (state_) {
            case state::initializing:
                return handle_processor_state_initializing(self);
            case state::reading:
                return handle_processor_state_reading(self, ec_in, bytes_transferred);
            case state::processing:
                return handle_processor_state_processing(self, ec_in);
            case state::done:
                [[fallthrough]];
            default:
                [[unlikely]] break; //Unreachable due to early check
        } //switch (state_)
    } //stream::input_processor::operator(Self&, std::error_code, std::size_t)

    /**
     * @internal
     * In `initializing`, calls `next_layer_.async_read_some` with `buffers_` unless `context_.input_side_buffer` already has data. Transitions to `reading`.
     * @remark Directly calls `handle_processor_state_reading` if there is data in the buffer already waiting to be processed.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    template<typename Self>
    void stream<NLS, PC>::input_processor<MBS>::handle_processor_state_initializing(Self& self)
    {
        state_ = state::reading;
        if (context_.input_side_buffer.size() == 0) {
            if (context_.deferred_transport_error) {
                //Immediately propagate a deferred error without attempting the next read.
                complete(self, std::exchange(context_.deferred_transport_error, {}), 0);
                return;
            }

            parent_stream_.launch_wait_for_urgent_data();

            auto read_buffer = context_.input_side_buffer.prepare(input_processor::read_block_size);
            parent_stream_.next_layer()
                .async_read_some(read_buffer, asio::bind_executor(parent_stream_.get_executor(), std::move(self)));
            return; //Wait for next_layer async_read_some to complete.
        }
        //If the buffer already has data, there is no need to wait for a network read.
        return handle_processor_state_reading(self);
    } //stream::input_processor::handle_processor_state_initializing(Self&)

    /**
     * @internal
     * In `reading`, sets up iterators (`user_buf_begin_`, `user_buf_end_`, `write_it_`) and transitions to `processing`.
     * @remark Directly calls `handle_processor_state_processing` to immediately begin processing.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    template<typename Self>
    void stream<NLS, PC>::input_processor<MBS>::handle_processor_state_reading(
        Self& self,
        std::error_code ec_in,
        std::size_t bytes_transferred
    )
    {
        context_.input_side_buffer.commit(bytes_transferred);

        if (context_.input_side_buffer.size() == 0) {
            //If there is no data to process, we can complete, propagating any read error.
            complete(self, ec_in, 0);
            return;
        }

        if (ec_in) {
            //If we had a deferred error, we skipped the read in state::initializing.
            //If we have a read error, defer it.
            context_.deferred_transport_error = ec_in;
        }

        //Set up private member iterators into the provided buffer.
        user_buf_begin_ = asio::buffers_begin(buffers_);
        user_buf_end_   = asio::buffers_end(buffers_);
        write_it_       = user_buf_begin_;

        state_ = state::processing;
        return handle_processor_state_processing(self);
    } //stream::input_processor::handle_processor_state_reading(Self&, std::error_code, std::size_t)

    /**
     * @internal
     * In `processing`, iterates over the buffer, calling `fsm_.process_byte`, handling forward flags (outside of urgent/Synch mode), and dispatching to `do_response` overloads that handle `ProcessingReturnVariant` responses via `std::visit`
     * Consumes bytes from the `context_.input_side_buffer` manually before async operations to avoid re-processing.
     * Completes with `std::distance(user_buf_begin_, write_it_)` bytes when `read_it == buf_end` or `write_it_ == user_buf_end_` or `process_byte` returns an error code. [std::distance should be linear time in the number of buffers in the sequence rather than the number of bytes]
     * Delegates to `process_fsm_signal` to handle `processing_signal`s returned from `process_byte`.
     * @remark Handles AO by clearing `context_.output_side_buffer` but propagates the signal to the caller for higher-level notification.
     * @note Unhandled `processing_signal`s and other `error_code`s propagate to the caller for higher-level notification.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    template<typename Self>
    void stream<NLS, PC>::input_processor<MBS>::handle_processor_state_processing(Self& self, std::error_code ec_in)
    {
        if (ec_in) { //This has to be a write error.
            process_write_error(ec_in);
        }

        std::error_code result_ec = std::exchange(context_.deferred_processing_signal, {});

        std::size_t bytes_consumed    = 0;
        std::size_t bytes_to_transfer = 0;

        if (!result_ec) {
            const auto proc_buffer = context_.input_side_buffer.data();
            auto buf_begin         = asio::buffers_begin(proc_buffer);
            auto buf_end           = asio::buffers_end(proc_buffer);
            auto read_it           = buf_begin;

            for (; (read_it != buf_end) && (write_it_ != user_buf_end_); ++read_it) {
                auto [proc_ec, forward, response] = fsm_.process_byte(static_cast<byte_t>(*read_it));

                if (proc_ec == processing_signal::abort_output) {
                    //AO clears the output side buffer.
                    context_.output_side_buffer.consume(context_.output_side_buffer.size());

                    //Defer the AO processing signal for application-level notification after the `async_send_synch` completes.
                    context_.deferred_processing_signal = proc_ec;

                    //Consume the bytes currently processed from the buffer INCLUDING this one
                    //MUST call now because we `move(self)` while handling `response` and lose access to `context_.input_side_buffer`
                    bytes_consumed = std::distance(buf_begin, read_it) + 1;
                    context_.input_side_buffer.consume(bytes_consumed);

                    parent_stream_.async_send_synch(std::move(self));
                    return;           //Wait for the asynchronous operation to complete.
                } else if (proc_ec) { //proc_ec will be cleared for non-terminal signals handled internally.
                    process_fsm_signal(proc_ec);
                }

                //Write a `forward`ed byte into the user's buffer.
                if (forward && !context_.urgent_data_state) {
                    *write_it_++ = *read_it;
                }
                if (proc_ec) { //Terminal signal or error
                    //Consume the bytes currently processed from the buffer INCLUDING this one
                    bytes_consumed = std::distance(buf_begin, read_it) + 1;
                    result_ec      = proc_ec; //Propagate the processing error_code
                    break;                    //Complete the operation.
                }
                if (response) {
                    //Consume the bytes currently processed from the buffer INCLUDING this one
                    //MUST call now because we `move(self)` while handling `response` and lose access to `context_.input_side_buffer`
                    bytes_consumed = std::distance(buf_begin, read_it) + 1;
                    context_.input_side_buffer.consume(bytes_consumed);

                    std::visit(
                        [this, self = std::move(self)](auto&& arg) mutable {
                            this->do_response(std::forward<decltype(arg)>(arg), std::move(self));
                        },
                        *response
                    );
                    return; //Wait for async operation to complete
                } //if (response)
            } //for
            if (!result_ec) {
                //SUCCESS: We either reached the end of the read data or filled the user's buffer.
                bytes_consumed = std::distance(buf_begin, read_it);

                //Since there is no current error, swap in any deferred error to report it.
                std::swap(context_.deferred_transport_error, result_ec);
            } //if (!result_ec) late check
        } //if (!result_ec) early check

        bytes_to_transfer = std::distance(user_buf_begin_, write_it_);
        context_.input_side_buffer.consume(bytes_consumed);

        if (!result_ec && (bytes_to_transfer == 0)) {
            //Re-initialize the processor for another underlying read if we haven't written into the user's buffer.
            return handle_processor_state_initializing(self);
        }

        complete(self, result_ec, bytes_to_transfer);
    } //stream::input_processor::handle_processor_state_processing(Self&, std::error_code)

    /**
     * @internal
     * Sets `state_` to `done` and calls `self.complete` with `ec` and `bytes_transferred`.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    template<typename Self>
    void stream<NLS, PC>::input_processor<MBS>::complete(Self& self, const std::error_code& ec, std::size_t bytes_transferred)
    {
        state_ = state::done;
        self.complete(ec, bytes_transferred);
    } //stream::input_processor::complete(Self&, const std::error_code&, std::size_t)

    /**
     * @internal
     * Defers write errors for later reporting and logs multi-error pile-ups.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    void stream<NLS, PC>::input_processor<MBS>::process_write_error(std::error_code ec)
    {
        if (context_.deferred_transport_error) {
            //We have a new write error on top of a previously deferred error.
            //Log it and attempt to continue processing the buffered byte stream.
            fsm_type::protocol_config_type::log_error(
                ec,
                "Error writing Telnet response with error {} previously deferred " "for reporting after processing the buffered byte stream.",
                context_.deferred_transport_error
            );
        } else {
            //Defer the write error.
            context_.deferred_transport_error = ec;
        }
    } //stream::input_processor::process_write_error(std::error_code)

    /*
     * @internal
     * Handles `processing_signal`s returned from `process_byte`.
     * @remark Handles EC and EL internally if there is buffered output to edit; otherwise, signals caller.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    void stream<NLS, PC>::input_processor<MBS>::process_fsm_signals(std::error_code& signal_ec)
    {
        //Handle `processing_signal`s that modify the buffer directly.
        if (signal_ec == processing_signal::carriage_return) {
            //A previously discarded '\r' byte must be inserted into the user's buffer.
            *write_it_++ = '\r';
            signal_ec.clear(); //Further processing is clear to continue.
        } else if ((signal_ec == processing_signal::erase_character) && (write_it_ != user_buf_begin_)) {
            //If the user buffer has data, EC backs up the write position by one character.
            //Otherwise, it propagates to the caller.
            --write_it_;
            signal_ec.clear(); //Further processing is clear to continue.
        } else if ((signal_ec == processing_signal::erase_line) && (write_it_ != user_buf_begin_)) {
            //If the user buffer has data, EL resets the write position to the beginning of the buffer.
            //Otherwise, it propagates to the caller.
            write_it_ = user_buf_begin_;
            signal_ec.clear(); //Further processing is clear to continue.
        } else if (signal_ec == processing_signal::data_mark) {
            context_.urgent_data_state.saw_data_mark();
            parent_stream_.launch_wait_for_urgent_data();
            signal_ec.clear(); //Further processing is clear to continue.
        }
    } //stream::input_processor::process_fsm_signal(std::error_code)

    /**
     * @internal
     * Initiates an asynchronous write of a `negotiation_response` using `async_write_negotiation`.
     * @see "net.telnet-stream.cppm" for interface, `:protocol_fsm` for `negotiation_response`, `:errors` for error codes, RFC 855 for negotiation
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    template<typename Self>
    void stream<NLS, PC>::input_processor<MBS>::do_response(
        typename stream::fsm_type::negotiation_response response,
        Self&& self
    )
    {
        parent_stream_.async_write_negotiation(response, std::forward<Self>(self));
    } //stream::input_processor::do_response(negotiation_response, Self&&)

    /**
     * @internal
     * Initiates an asynchronous write of raw data using `async_write_raw`.
     * @remark Wraps the `std::string` in a `std::shared_ptr<std::string>` for lifetime management and forwards it to `async_write_raw`.
     * @see "net.telnet-stream.cppm" for interface, `:errors` for error codes, RFC 854 for IAC escaping
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    template<typename Self>
    void stream<NLS, PC>::input_processor<MBS>::do_response(std::string response, Self&& self)
    {
        static std::string resp = std::move(response);
        parent_stream_.async_write_raw(asio::buffer(resp), std::forward<Self>(self));
    } //stream::input_processor::do_response(std::string, Self&&)

    /**
     * @internal
     * Spawns a coroutine to process a `subnegotiation_awaitable`, writing the result via `async_write_subnegotiation` if non-empty.
     * @remark Uses `asio::co_spawn` to execute the awaitable, handling potential exceptions by throwing `std::system_error` with `telnet::error::internal_error` for non-system errors.
     * @see "net.telnet-stream.cppm" for interface, `:awaitables` for `subnegotiation_awaitable`, `:errors` for error codes, RFC 855 for subnegotiation
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    template<typename Self>
    void stream<NLS, PC>::input_processor<MBS>::do_response(awaitables::subnegotiation_awaitable awaitable, Self&& self)
    {
        asio::co_spawn(
            parent_stream_.get_executor(),
            //NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines): Lambda closure lifetime is ensured by Asio. `this` lifetime is bound to parent operation which will not continue until the coroutine returns.
            [this, handler_awaitable = std::move(awaitable)]() mutable -> asio::awaitable<std::size_t> {
                try {
                    auto [opt, subneg_buffer] = co_await handler_awaitable;
                    if (!subneg_buffer.empty()) {
                        co_return co_await parent_stream_.async_write_subnegotiation(opt, subneg_buffer, asio::use_awaitable);
                    }
                    co_return std::size_t{0};
                } catch (const std::system_error& se) {
                    throw;
                } catch (...) {
                    throw std::system_error(error::internal_error);
                }
            },
            std::forward<Self>(self)
        );
    } //stream::input_processor::do_response(awaitables::subnegotiation_awaitable, Self&&)

    /**
     * @internal
     * Spawns a coroutine to process a `tagged_awaitable`, optionally writing a `negotiation_response` via `async_write_negotiation`.
     * @remark Uses `asio::co_spawn` to execute the awaitable, followed by an optional negotiation write, handling exceptions by throwing `std::system_error` with `telnet::error::internal_error` for non-system errors.
     * @see "net.telnet-stream.cppm" for interface, `:awaitables` for `tagged_awaitable`, `:protocol_fsm` for `negotiation_response`, `:errors` for error codes, RFC 855 for negotiation
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBS>
    template<typename Self, typename Tag, typename T, typename Awaitable>
    void stream<NLS, PC>::input_processor<MBS>::do_response(
        std::tuple<
            awaitables::tagged_awaitable<Tag, T, Awaitable>,
            std::optional<typename stream::fsm_type::negotiation_response>
        > response,
        Self&& self
    )
    {
        auto [awaitable, negotiation] = std::move(response);
        asio::co_spawn(
            parent_stream_.get_executor(),
            //NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines): Lambda closure lifetime is ensured by Asio. `this` lifetime is bound to parent operation which will not continue until the coroutine returns.
            [this,
             awaitable   = std::move(awaitable),
             negotiation = std::move(negotiation)]() mutable -> asio::awaitable<std::size_t> {
                try {
                    std::size_t bytes_transferred = 0;
                    if (negotiation) {
                        bytes_transferred += co_await parent_stream_.async_write_negotiation(*negotiation, asio::use_awaitable);
                    }
                    co_await awaitable;
                    co_return bytes_transferred;
                } catch (const std::system_error& se) {
                    throw;
                } catch (...) {
                    throw std::system_error(error::internal_error);
                }
            },
            std::forward<Self>(self)
        );
    } //stream::input_processor::do_response(tagged_awaitable<Tag, T, Awaitable>, Self&&)
} //namespace net::telnet
