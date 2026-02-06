// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
/**
 * @file net.telnet-stream-async-impl.cpp
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
 * @brief Implementation of asynchronous Telnet stream operations.
 * @remark Contains implementations for `async_read_some`, `async_write_some`, `async_write_raw`, `async_write_command`, `async_write_negotiation`, `async_write_subnegotiation`, `async_write_temp_buffer`, `async_report_error`, `async_request_option`, `async_disable_option`, and `async_send_synch`.
 *
 * @see "net.telnet-stream.cppm" for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, `:types` for `telnet::command`, `:options` for `option` and `option::id_num`, `:errors` for error codes, `:protocol_fsm` for `ProtocolFSM`
 */

module; //Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio.hpp>

//Module implementation unit
module net.telnet;

import std; //NOLINT For std::array, std::vector, std::ignore

import :types;        ///< @see "net.telnet-types.cppm" for `telnet::command`
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
     * Implements asynchronous option request by delegating to FSM and writing negotiation.
     * @remark Calls `fsm_.request_option` to check state and generate a response tuple containing an optional `negotiation_response`.
     * @remark If no response is needed or an error occurs, invokes `async_report_error` with the error code and 0 bytes.
     * @remark Forwards valid responses to `async_write_negotiation` to send IAC WILL/DO sequence.
     * @remark Uses `std::forward` for `CompletionToken` to preserve handler efficiency.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<typename CompletionToken>
    auto stream<NLS, PC>::async_request_option(option::id_num opt, negotiation_direction direction, CompletionToken&& token)
    {
        auto [ec, response] = fsm_.request_option(opt, direction);
        if (ec || !response) {
            return async_report_error(ec, std::forward<CompletionToken>(token));
        } else {
            return async_write_negotiation(*response, std::forward<CompletionToken>(token));
        }
    } //stream::async_request_option(option::id_num, negotiation_direction, CompletionToken&&)

    /**
     * @internal
     * Implements asynchronous option disablement with FSM state management and optional awaitable handling.
     * @remark Calls `fsm_.disable_option` to obtain error code, optional `negotiation_response`, and optional awaitable.
     * @remark Uses `asio::co_spawn` to manage coroutine lifetime, executing on the stream’s executor.
     * @remark If a response is present, writes IAC WONT/DONT via `async_write_negotiation` and accumulates bytes transferred.
     * @remark If an awaitable is present, co_awaits it to handle registered disablement callbacks.
     * @remark Catches non-system errors and converts to `telnet::error::internal_error` for consistency.
     * @remark Returns 0 bytes on error via `async_report_error` if neither response nor awaitable is provided.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<typename CompletionToken>
    auto stream<NLS, PC>::async_disable_option(option::id_num opt, negotiation_direction direction, CompletionToken&& token)
    {
        auto [ec, response, awaitable] = fsm_.disable_option(opt, direction);
        if (ec || (!response && !awaitable)) {
            return async_report_error(ec, std::forward<CompletionToken>(token));
        } else {
            return asio::co_spawn(
                this->get_executor(),
                //NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines): Lambda closure lifetime is ensured by Asio. `this` lifetime is bound to parent stream and must be guaranteed for the operation to complete.
                [this,
                 response  = std::move(response),
                 awaitable = std::move(awaitable)]() mutable -> asio::awaitable<std::size_t> {
                    try {
                        std::size_t bytes_transferred = 0;
                        if (response) {
                            bytes_transferred += co_await async_write_negotiation(*response, asio::use_awaitable);
                        }
                        if (awaitable) {
                            co_await *awaitable;
                        }
                        co_return bytes_transferred;
                    } catch (std::system_error& sys_err) {
                        throw;
                    } catch (...) {
                        throw std::system_error(error::internal_error);
                    }
                },
                std::forward<CompletionToken>(token)
            );
        }
    } //stream::async_disable_option(option::id_num, negotiation_direction, CompletionToken&&)

    /**
     * @internal
     * Uses `asio::async_compose` to initiate asynchronous read with `input_processor`.
     * @remark Binds the operation to the stream’s executor using `get_executor()`.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<MutableBufferSequence MBufSeq, ReadToken CompletionToken>
    auto stream<NLS, PC>::async_read_some(const MBufSeq& buffers, CompletionToken&& token)
    {
        return asio::async_compose<CompletionToken, typename stream<NLS, PC>::asio_completion_signature>(
            input_processor<MBufSeq>(*this, fsm_, context_, std::move(buffers)),
            std::forward<CompletionToken>(token),
            this->get_executor()
        );
    } //stream::async_read_some(MutableBufferSequence, CompletionToken&&)

    /**
     * @internal
     * Escapes input data using `escape_telnet_output` and delegates to `async_write_temp_buffer`.
     * @remark Returns via `async_report_error` if `escape_telnet_output` fails.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<ConstBufferSequence CBufSeq, WriteToken CompletionToken>
    auto stream<NLS, PC>::async_write_some(const CBufSeq& data, CompletionToken&& token)
    {
        auto [ec, escaped_data] = escape_telnet_output(data);
        if (ec) {
            return async_report_error(ec, std::forward<CompletionToken>(token));
        }
        return async_write_temp_buffer(std::move(escaped_data), std::forward<CompletionToken>(token));
    } //stream::async_write_some(const CBufSeq&, CompletionToken&&)

    /**
     * @internal
     * Uses `asio::async_write` to write the raw buffer directly to `next_layer_`.
     * @remark Binds the operation to the stream’s executor using `asio::bind_executor`.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<ConstBufferSequence CBufSeq, WriteToken CompletionToken>
    auto stream<NLS, PC>::async_write_raw(const CBufSeq& data, CompletionToken&& token)
    {
        return asio::async_write(
            this->next_layer_, data, asio::bind_executor(this->get_executor(), std::forward<CompletionToken>(token))
        );
    } //stream::async_write_raw(const CBufSeq&, CompletionToken&&)

    /**
     * @internal
     * Constructs a 2-byte `std::array` with `{IAC, cmd}` and writes it using `asio::async_write`.
     * @remark Binds the operation to the stream’s executor using `asio::bind_executor`.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<WriteToken CompletionToken>
    auto stream<NLS, PC>::async_write_command(telnet::command cmd, CompletionToken&& token)
    {
        static std::array<byte_t, 2> buf{std::to_underlying(telnet::command::iac), std::to_underlying(cmd)};

        return asio::async_write(
            this->next_layer_,
            asio::buffer(buf),
            asio::bind_executor(this->get_executor(), std::forward<CompletionToken>(token))
        );
    } //stream::async_write_command(telnet::command, CompletionToken&&)

    /**
     * @internal
     * Validates `opt` using `opt.supports_subnegotiation()` and `fsm_.is_enabled(opt)`.
     * @remark Reserves space in `escaped_buffer` for subnegotiation data plus 5 bytes (IAC SB, opt, IAC SE).
     * @remark Appends IAC SB, `opt`, escaped subnegotiation data via `escape_telnet_output`, and IAC SE.
     * @remark Returns `telnet::error::invalid_subnegotiation` or `telnet::error::option_not_available` via `async_report_error` if validation fails.
     * @remark Catches `std::bad_alloc` to return `std::errc::not_enough_memory` and other exceptions to return `telnet::error::internal_error` via `async_report_error`.
     * @remark Delegates to `async_write_temp_buffer` for writing the constructed buffer.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<WriteToken CompletionToken>
    auto stream<NLS, PC>::async_write_subnegotiation(
        option opt,
        const std::vector<byte_t>& subnegotiation_buffer,
        CompletionToken&& token
    )
    {
        if (!opt.supports_subnegotiation()) {
            return async_report_error(make_error_code(error::invalid_subnegotiation), std::forward<CompletionToken>(token));
        }
        if (!fsm_.is_enabled(opt)) {
            return async_report_error(make_error_code(error::option_not_available), std::forward<CompletionToken>(token));
        }

        std::vector<byte_t> escaped_buffer;
        try {
            //Reserve space for the original size plus 10% for escaping and 5 bytes for framing (IAC SB, opt, IAC SE)
            constexpr double escaping_cushion_factor = 1.1;
            using size_type                          = decltype(subnegotiation_buffer.size());
            constexpr size_type framing_padding      = 5;
            escaped_buffer.reserve(
                static_cast<size_type>(static_cast<double>(subnegotiation_buffer.size()) * escaping_cushion_factor)
                + framing_padding
            );

            //Append initial framing: IAC SB opt
            escaped_buffer.push_back(std::to_underlying(telnet::command::iac));
            escaped_buffer.push_back(std::to_underlying(telnet::command::sb));
            escaped_buffer.push_back(std::to_underlying(opt.get_id()));

            //Escape the subnegotiation data
            std::error_code ec;
            std::ignore = escape_telnet_output(escaped_buffer, subnegotiation_buffer);
            if (ec) {
                return async_report_error(ec, std::forward<CompletionToken>(token));
            }

            //Append final framing: IAC SE
            escaped_buffer.push_back(std::to_underlying(telnet::command::iac));
            escaped_buffer.push_back(std::to_underlying(telnet::command::se));
        } catch (const std::bad_alloc&) {
            return async_report_error(make_error_code(std::errc::not_enough_memory), std::forward<CompletionToken>(token));
        } catch (...) {
            return async_report_error(make_error_code(error::internal_error), std::forward<CompletionToken>(token));
        }

        return async_write_temp_buffer(std::move(escaped_buffer), std::forward<CompletionToken>(token));
    } //stream::async_write_subnegotiation(option, const std::vector<byte_t>&, CompletionToken&&)

    /**
     * @internal
     * Send 3 NULs, middle urgent, then IAC DM.
     * Ensures correct Synch behavior regardless of URG pointer semantics.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<WriteToken CompletionToken>
    auto stream<NLS, PC>::async_send_synch(CompletionToken&& token)
    {
        return asio::async_initiate<CompletionToken, asio_completion_signature>(
            [this](auto handler) {
                std::size_t bytes_transferred = 0;
                this->async_send_nul( //Send 1
                    false,
                    [this, bytes_transferred, handler = std::move(handler)](std::error_code ec,
                                                                            std::size_t bytes) mutable {
                        bytes_transferred += bytes;
                        if (ec) {
                            asio::dispatch(std::move(handler), ec, bytes_transferred);
                            return;
                        }
                        this->async_send_nul( //Send 2
                            true,             //Urgent/OOB
                            [this, bytes_transferred, handler = std::move(handler)](std::error_code ec,
                                                                                    std::size_t bytes) mutable {
                                bytes_transferred += bytes;
                                if (ec) {
                                    asio::dispatch(std::move(handler), ec, bytes_transferred);
                                    return;
                                }
                                this->async_send_nul( //Send 3
                                    false,
                                    [this, bytes_transferred, handler = std::move(handler)](std::error_code ec,
                                                                                            std::size_t bytes) mutable {
                                        bytes_transferred += bytes;
                                        if (ec) {
                                            asio::dispatch(std::move(handler), ec, bytes_transferred);
                                            return;
                                        }
                                        this->async_write_command( //Final IAC DM
                                            telnet::command::dm,
                                            [bytes_transferred,
                                             handler = std::move(handler)](std::error_code ec,
                                                                           std::size_t bytes) mutable {
                                                bytes_transferred += bytes;
                                                asio::dispatch(std::move(handler), ec, bytes_transferred);
                                            }); //End of final IAC DM
                                    });         //End of send 3
                            });                 //End of send 2
                    });                         //End of send 1
            },
            std::forward<CompletionToken>(token)
        ); //End of async_initiate
    } //stream::async_send_synch(CompletionToken&&)

    /**
     * @internal
     * Holds a static NUL byte and `async_send`s it (with `message_out_of_band` set if urgent).
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<WriteToken CompletionToken>
    auto stream<NLS, PC>::async_send_nul(bool urgent, CompletionToken&& token)
    {
        constexpr static auto nul = static_cast<byte_t>('\0');

        if (urgent) {
            return this->lowest_layer().async_send(
                asio::buffer(&nul, 1), lowest_layer_type::message_out_of_band, std::forward<CompletionToken>(token)
            );
        } else {
            return this->lowest_layer().async_send(asio::buffer(&nul, 1), std::forward<CompletionToken>(token));
        }
    } //stream::async_send_nul(bool, CompletionToken&&)

    /**
     * @internal
     * @remark Constructs a 3-byte `std::array` with `{IAC, cmd, opt}` and writes it using `asio::async_write` if valid.
     * @remark Binds the operation to the stream’s executor using `asio::bind_executor`.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<WriteToken CompletionToken>
    auto stream<NLS, PC>::async_write_negotiation(typename fsm_type::negotiation_response response, CompletionToken&& token)
    {
        auto [dir, enable, opt] = response;
        static std::array<byte_t, 3>
            buf{std::to_underlying(telnet::command::iac),
                std::to_underlying(fsm_type::make_negotiation_command(dir, enable)),
                std::to_underlying(opt)};
        return asio::async_write(
            this->next_layer_,
            asio::buffer(buf),
            asio::bind_executor(this->get_executor(), std::forward<CompletionToken>(token))
        );
    } //stream::async_write_negotiation(fsm_type::negotiation_response, CompletionToken&&)

    /**
     * @internal
     * Uses `asio::async_initiate` to write `temp_buffer` via `asio::async_write` to `next_layer_`.
     * @remark Moves `temp_buffer` into a lambda to manage its lifetime.
     * @remark Binds the operation to the stream’s executor using `asio::bind_executor`.
     * @remark Forwards the handler to receive `ec` and `bytes_written`.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<WriteToken CompletionToken>
    auto stream<NLS, PC>::async_write_temp_buffer(std::vector<byte_t>&& temp_buffer, CompletionToken&& token)
    {
        return asio::async_initiate<CompletionToken, typename stream<NLS, PC>::asio_completion_signature>(
            [this, temp_buffer = std::move(temp_buffer)](auto handler) mutable {
                asio::async_write(
                    this->next_layer_,
                    asio::buffer(temp_buffer),
                    asio::bind_executor(
                        this->get_executor(),
                        [handler](const std::error_code& ec, std::size_t bytes_written) mutable { handler(ec, bytes_written); }
                    )
                );
            },
            std::forward<CompletionToken>(token)
        );
    } //stream::async_write_temp_buffer(std::vector<byte_t>&&, CompletionToken&&)

    /**
     * @internal
     * Uses `asio::async_initiate` to invoke the handler with `ec` and 0 bytes transferred.
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    template<WriteToken CompletionToken>
    auto stream<NLS, PC>::async_report_error(std::error_code ec, CompletionToken&& token)
    {
        return asio::async_initiate<CompletionToken, typename stream<NLS, PC>::asio_completion_signature>(
            [ec](auto handler) { handler(ec, 0); }, std::forward<CompletionToken>(token)
        );
    } //stream::async_report_error(std::error_code, CompletionToken&&)
} //namespace net::telnet
