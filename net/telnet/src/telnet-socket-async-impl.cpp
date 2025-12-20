/**
 * @file telnet-socket-async-impl.cpp
 * @version 0.2.0
 * @release_date September 19, 2025
 * @brief Implementation of asynchronous Telnet socket operations.
 * @remark Contains async_read_some, async_write_some, async_write_command, async_write_negotiation, async_write_subnegotiation, async_write_temp_buffer, and async_report_error.
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 * @see telnet-socket.cppm for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for error codes, telnet:protocol_fsm for ProtocolFSM
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition implementation unit
module telnet:socket;

import std;        // For std::array, std::vector, std::ignore
import std.compat; // For std::uint8_t 

import :errors;       ///< @see telnet-errors.cppm for telnet::error codes
import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm for ProtocolFSM
import :options;      ///< @see telnet-options.cppm for option::id_num

import :socket;       ///< @see telnet-socket.cppm for socket partition interface

namespace telnet {
    /**
     * @brief Asynchronously reads some data with Telnet processing.
     * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
     * @tparam CompletionToken The type of completion token.
     * @param buffers The mutable buffer sequence to read into.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Initiates a read from the underlying layer and processes Telnet data using async_compose.
     * @see telnet-socket.cppm for interface, InputProcessor for processing details, telnet-protocol_fsm.cppm for ProtocolFSM, telnet:errors for error codes
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
    } //socket::async_read_some(MutableBufferSequence, CompletionToken&&)

    /**
     * @brief Asynchronously writes some data with Telnet-specific IAC escaping.
     * @param data The data to write (string or binary).
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Uses the next layer for I/O to respect protocol layering and applies `escape_telnet_output` for IAC escaping.
     * @see telnet-socket.cppm for interface, escape_telnet_output for escaping details, telnet:errors for error codes, RFC 854 for IAC escaping
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
    } //socket::async_write_some(const ConstBufferSequence&, CompletionToken&&)

    /**
     * @brief Asynchronously writes a Telnet command.
     * @param cmd The Telnet command to send.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Constructs a buffer with {IAC, static_cast<std::uint8_t>(cmd)} and uses the next layer for I/O to respect protocol layering.
     * @see telnet-socket.cppm for interface, telnet:types for TelnetCommand, telnet:errors for error codes, RFC 854 for command structure
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_write_command(TelnetCommand cmd, CompletionToken&& token)
        -> typename socket<NLS, PC>::template asio_result_type<CompletionToken>
    {
        return asio::async_write(
            this->next_layer_,
            asio::buffer(std::array<std::uint8_t, 2>{
                static_cast<std::uint8_t>(TelnetCommand::IAC),
                static_cast<std::uint8_t>(cmd)
            }),
            asio::bind_executor(this->get_executor(), std::forward<CompletionToken>(token))
        );
    } //socket::async_write_command(TelnetCommand, CompletionToken&&)

    /**
     * @brief Asynchronously writes a Telnet negotiation command with an option.
     * @param cmd The Telnet command to send.
     * @param opt The option::id_num to send.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Constructs a buffer with {IAC, static_cast<std::uint8_t>(cmd), static_cast<std::uint8_t>(opt)} and uses the next layer for I/O to respect protocol layering.
     * @remark Sets error code to `telnet::error::invalid_negotiation` if cmd is not WILL, WONT, DO, or DONT.
     * @see telnet-socket.cppm for interface, telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for invalid_negotiation, RFC 855 for negotiation
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
            asio::buffer(std::array<std::uint8_t, 3>{
                static_cast<std::uint8_t>(TelnetCommand::IAC),
                static_cast<std::uint8_t>(cmd),
                static_cast<std::uint8_t>(opt)
            }),
            asio::bind_executor(this->get_executor(), std::forward<CompletionToken>(token))
        );
    } //socket::async_write_negotiation(TelnetCommand, option::id_num, CompletionToken&&)

    /**
     * @brief Asynchronously writes a Telnet subnegotiation command.
     * @param opt The option::id_num for the subnegotiation.
     * @param subnegotiation_buffer The data buffer for the subnegotiation.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Validates that the option is enabled via FSM, reserves space for escaped data plus 5 bytes (IAC SB, opt, IAC SE), appends IAC SB opt, calls `escape_telnet_output` to append the escaped data, and appends IAC SE, with error handling for memory allocation.
     * @remark Uses ProtocolFSM to check if the option::id_num is enabled and supports subnegotiation, registering a default option if unregistered.
     * @note Subnegotiation overflow is handled by ProtocolFSM::process_byte for incoming subnegotiations, not outgoing writes.
     * @see telnet-socket.cppm for interface, telnet:options for option::id_num, telnet:errors for error codes, escape_telnet_output for escaping, telnet-protocol_fsm.cppm for ProtocolFSM, RFC 855 for subnegotiation
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_write_subnegotiation(option::id_num opt, const std::vector<std::uint8_t>& subnegotiation_buffer, CompletionToken&& token)
        -> typename socket<NLS, PC>::template asio_result_type<CompletionToken>
    {
        if (!fsm_.is_enabled(opt)) {
            return async_report_error(std::make_error_code(error::option_not_available), std::forward<CompletionToken>(token));
        }

        std::vector<std::uint8_t> escaped_buffer;
        try {
            // Reserve space for the original size plus 10% for escaping and 5 bytes for framing (IAC SB, opt, IAC SE)
            escaped_buffer.reserve(subnegotiation_buffer.size() * 1.1 + 5);

            // Append initial framing: IAC SB opt
            escaped_buffer.push_back(static_cast<std::uint8_t>(TelnetCommand::IAC));
            escaped_buffer.push_back(static_cast<std::uint8_t>(TelnetCommand::SB));
            escaped_buffer.push_back(static_cast<std::uint8_t>(opt));

            // Escape the subnegotiation data
            std::error_code ec;
            std::tie(ec, std::ignore) = escape_telnet_output(escaped_buffer, subnegotiation_buffer);
            if (ec) {
                return async_report_error(ec, std::forward<CompletionToken>(token));
            }

            // Append final framing: IAC SE
            escaped_buffer.push_back(static_cast<std::uint8_t>(TelnetCommand::IAC));
            escaped_buffer.push_back(static_cast<std::uint8_t>(TelnetCommand::SE));
        } catch (const std::bad_alloc&) {
            return async_report_error(std::make_error_code(std::errc::not_enough_memory), std::forward<CompletionToken>(token));
        } catch (...) {
            return async_report_error(std::make_error_code(error::internal_error), std::forward<CompletionToken>(token));
        }

        return async_write_temp_buffer(std::move(escaped_buffer), std::forward<CompletionToken>(token));
    } //socket::async_write_subnegotiation(option::id_num, const std::vector<std::uint8_t>&, CompletionToken&&)

    /**
     * @brief Asynchronously writes a temporary buffer to the next layer.
     * @param temp_buffer The temporary buffer to write.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Used by async_write_some, async_write_command, async_write_negotiation, and async_write_subnegotiation to write variable-sized prepared buffers.
     * @remark Monitor performance to re-evaluate buffering strategy if this becomes a bottleneck.
     * @see telnet-socket.cppm for interface, async_write_some, async_write_subnegotiation, RFC 854 for IAC escaping
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename CompletionToken>
      requires asio::completion_token_for<CompletionToken, typename socket<NLS, PC>::asio_completion_signature>
    auto socket<NLS, PC>::async_write_temp_buffer(std::vector<std::uint8_t>&& temp_buffer, CompletionToken&& token)
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
    } //socket::async_write_temp_buffer(std::vector<std::uint8_t>&&, CompletionToken&&)

    /**
     * @brief Asynchronously reports an error via the completion token.
     * @param ec The error code to report.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Used to report errors asynchronously in async_write_some, async_write_command, async_write_negotiation, and async_write_subnegotiation.
     * @see telnet-socket.cppm for interface, telnet:errors for error codes
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
    } //socket::async_report_error(std::error_code, CompletionToken&&)
} //namespace telnet