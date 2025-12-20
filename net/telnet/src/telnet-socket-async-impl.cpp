/**
 * @file telnet-socket-async-impl.cpp
 * @version 0.4.0
 * @release_date October 3, 2025
 *
 * @brief Implementation of asynchronous Telnet socket operations.
 * @remark Contains implementations for `async_read_some`, `async_write_some`, `async_write_raw`, `async_write_command`, `async_write_negotiation`, `async_write_subnegotiation`, `async_write_temp_buffer`, and `async_report_error`.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see :socket for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, :types for `TelnetCommand`, :options for `option` and `option::id_num`, :errors for error codes, :protocol_fsm for `ProtocolFSM`
 */
module; // Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

// Module partition implementation unit
module telnet:socket;

import std; // For std::array, std::vector, std::ignore

import :types;        ///< @see telnet-types.cppm for `TelnetCommand`
import :errors;       ///< @see telnet-errors.cppm for `telnet::error` codes
import :options;      ///< @see telnet-options.cppm for `option` and `option::id_num`
import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm for `ProtocolFSM`

import :socket;       ///< @see telnet-socket.cppm for socket partition interface

namespace telnet {
    /**
     * @internal
     * @brief Uses `asio::async_compose` to initiate asynchronous read with `InputProcessor`.
     * @remark Binds the operation to the socket’s executor using `get_executor()`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_read_some(MutableBufferSequence buffers, CompletionToken&& token)
        -> typename socket<NLS, PC>::template asio_result_type<CompletionToken>
    {
        return asio::async_compose<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>(
            InputProcessor<MutableBufferSequence>(*this, fsm_, std::move(buffers)),
            std::forward<CompletionToken>(token),
            this->get_executor());
    } // socket::async_read_some(MutableBufferSequence, CompletionToken&&)

    /**
     * @internal
     * @brief Escapes input data using `escape_telnet_output` and delegates to `async_write_temp_buffer`.
     * @remark Returns via `async_report_error` if `escape_telnet_output` fails.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_write_some(const ConstBufferSequence& data, CompletionToken&& token)
        -> typename socket<NLS, PC>::template asio_result_type<CompletionToken>
    {
        auto [ec, escaped_data] = escape_telnet_output(data);
        if (ec) {
            return async_report_error(ec, std::forward<CompletionToken>(token));
        }
        return async_write_temp_buffer(std::move(escaped_data), std::forward<CompletionToken>(token));
    } // socket::async_write_some(const ConstBufferSequence&, CompletionToken&&)

    /**
     * @internal
     * @brief Uses `asio::async_write` to write the raw buffer directly to `next_layer_`.
     * @remark Binds the operation to the socket’s executor using `asio::bind_executor`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_write_raw(const ConstBufferSequence& data, CompletionToken&& token)
        -> typename socket<NLS, PC>::template asio_result_type<CompletionToken>
    {
        return asio::async_write(
            this->next_layer_,
            data,
            asio::bind_executor(this->get_executor(), std::forward<CompletionToken>(token))
        );
    } // socket::async_write_raw(const ConstBufferSequence&, CompletionToken&&)

    /**
     * @internal
     * @brief Constructs a 2-byte `std::array` with `{IAC, cmd}` and writes it using `asio::async_write`.
     * @remark Binds the operation to the socket’s executor using `asio::bind_executor`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_write_command(TelnetCommand cmd, CompletionToken&& token)
        -> typename socket<NLS, PC>::template asio_result_type<CompletionToken>
    {
        return asio::async_write(
            this->next_layer_,
            asio::buffer(std::array<byte_t, 2>{
                std::to_underlying(TelnetCommand::IAC),
                std::to_underlying(cmd)
            }),
            asio::bind_executor(this->get_executor(), std::forward<CompletionToken>(token))
        );
    } // socket::async_write_command(TelnetCommand, CompletionToken&&)

    /**
     * @internal
     * @brief Validates that `cmd` is one of `WILL`, `WONT`, `DO`, or `DONT`.
     * @remark Constructs a 3-byte `std::array` with `{IAC, cmd, opt}` and writes it using `asio::async_write` if valid.
     * @remark Returns `telnet::error::invalid_negotiation` via `async_report_error` if `cmd` is invalid.
     * @remark Binds the operation to the socket’s executor using `asio::bind_executor`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_write_negotiation(TelnetCommand cmd, option::id_num opt, CompletionToken&& token)
        -> typename socket<NLS, PC>::template asio_result_type<CompletionToken>
    {
        if (cmd != TelnetCommand::WILL && cmd != TelnetCommand::WONT &&
            cmd != TelnetCommand::DO && cmd != TelnetCommand::DONT) {
            return async_report_error(std::make_error_code(error::invalid_negotiation), std::forward<CompletionToken>(token));
        }

        return asio::async_write(
            this->next_layer_,
            asio::buffer(std::array<byte_t, 3>{
                std::to_underlying(TelnetCommand::IAC),
                std::to_underlying(cmd),
                std::to_underlying(opt)
            }),
            asio::bind_executor(this->get_executor(), std::forward<CompletionToken>(token))
        );
    } // socket::async_write_negotiation(TelnetCommand, option::id_num, CompletionToken&&)

    /**
     * @internal
     * @brief Validates `opt` using `opt.supports_subnegotiation()` and `fsm_.is_enabled(opt)`.
     * @remark Reserves space in `escaped_buffer` for subnegotiation data plus 5 bytes (IAC SB, opt, IAC SE).
     * @remark Appends IAC SB, `opt`, escaped subnegotiation data via `escape_telnet_output`, and IAC SE.
     * @remark Returns `telnet::error::invalid_subnegotiation` or `telnet::error::option_not_available` via `async_report_error` if validation fails.
     * @remark Catches `std::bad_alloc` to return `std::errc::not_enough_memory` and other exceptions to return `telnet::error::internal_error` via `async_report_error`.
     * @remark Delegates to `async_write_temp_buffer` for writing the constructed buffer.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer, CompletionToken&& token)
        -> typename socket<NLS, PC>::template asio_result_type<CompletionToken>
    {
        if (!opt.supports_subnegotiation()) {
            return async_report_error(std::make_error_code(error::invalid_subnegotiation), std::forward<CompletionToken>(token));
        }
        if (!fsm_.is_enabled(opt)) {
            return async_report_error(std::make_error_code(error::option_not_available), std::forward<CompletionToken>(token));
        }

        std::vector<byte_t> escaped_buffer;
        try {
            // Reserve space for the original size plus 10% for escaping and 5 bytes for framing (IAC SB, opt, IAC SE)
            escaped_buffer.reserve(subnegotiation_buffer.size() * 1.1 + 5);

            // Append initial framing: IAC SB opt
            escaped_buffer.push_back(std::to_underlying(TelnetCommand::IAC));
            escaped_buffer.push_back(std::to_underlying(TelnetCommand::SB));
            escaped_buffer.push_back(std::to_underlying(opt));

            // Escape the subnegotiation data
            std::error_code ec;
            std::ignore = escape_telnet_output(escaped_buffer, subnegotiation_buffer);
            if (ec) {
                return async_report_error(ec, std::forward<CompletionToken>(token));
            }

            // Append final framing: IAC SE
            escaped_buffer.push_back(std::to_underlying(TelnetCommand::IAC));
            escaped_buffer.push_back(std::to_underlying(TelnetCommand::SE));
        } catch (const std::bad_alloc&) {
            return async_report_error(std::make_error_code(std::errc::not_enough_memory), std::forward<CompletionToken>(token));
        } catch (...) {
            return async_report_error(std::make_error_code(error::internal_error), std::forward<CompletionToken>(token));
        }

        return async_write_temp_buffer(std::move(escaped_buffer), std::forward<CompletionToken>(token));
    } // socket::async_write_subnegotiation(option, const std::vector<byte_t>&, CompletionToken&&)

    /**
     * @internal
     * @brief Uses `asio::async_initiate` to write `temp_buffer` via `asio::async_write` to `next_layer_`.
     * @remark Moves `temp_buffer` into a lambda to manage its lifetime.
     * @remark Binds the operation to the socket’s executor using `asio::bind_executor`.
     * @remark Forwards the handler to receive `ec` and `bytes_written`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_write_temp_buffer(std::vector<byte_t>&& temp_buffer, CompletionToken&& token)
        -> typename socket<NLS, PC>::template asio_result_type<CompletionToken>
    {
        return asio::async_initiate<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>(
            [this, temp_buffer = std::move(temp_buffer)](auto handler) mutable {
                asio::async_write(this->next_layer_,
                    asio::buffer(temp_buffer),
                    asio::bind_executor(this->get_executor(),
                        [handler](const std::error_code& ec, std::size_t bytes_written) mutable {
                            handler(ec, bytes_written);
                        })
                );
            }, std::forward<CompletionToken>(token));
    } // socket::async_write_temp_buffer(std::vector<byte_t>&&, CompletionToken&&)

    /**
     * @internal
     * @brief Uses `asio::async_initiate` to invoke the handler with `ec` and 0 bytes transferred.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_report_error(std::error_code ec, CompletionToken&& token)
        -> typename socket<NLS, PC>::template asio_result_type<CompletionToken>
    {
        return asio::async_initiate<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>(
            [ec](auto handler) {
                handler(ec, 0);
            }, std::forward<CompletionToken>(token));
    } // socket::async_report_error(std::error_code, CompletionToken&&)
} // namespace telnet