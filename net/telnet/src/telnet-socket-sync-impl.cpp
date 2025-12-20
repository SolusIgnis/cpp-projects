/**
 * @file telnet-socket-sync-impl.cpp
 * @version 0.2.0
 * @release_date September 19, 2025
 * @brief Implementation of synchronous Telnet socket operations.
 * @remark Contains read_some, write_some, write_command, write_negotiation, and write_subnegotiation, wrapping asynchronous counterparts.
 * @warning Synchronous I/O operations incur overhead from sync_await, which creates a new io_context and thread per call, and scale poorly compared to asynchronous methods. However, this overhead is minimal compared to blocking network I/O latency.
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 * @see telnet-socket.cppm for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for error codes, telnet:protocol_fsm for ProtocolFSM
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition implementation unit
module telnet:socket;

import std;        // For std::size_t, std::system_error
import std.compat; // For std::uint8_t

import :errors;       ///< @see telnet-errors.cppm for telnet::error codes
import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm for ProtocolFSM
import :options;      ///< @see telnet-options.cppm for option::id_num

import :socket;       ///< @see telnet-socket.cppm for socket partition interface

namespace telnet {
    /**
     * @remark Synchronous throwing wrappers call sync_await on their asynchronous counterparts and propagate exceptions.
     */
    /**
     * @brief Synchronously reads some data with Telnet processing.
     * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
     * @param buffers The mutable buffer sequence to read into.
     * @return The number of bytes read.
     * @remark Wraps async_read_some using sync_await, propagating exceptions.
     * @see telnet-socket.cppm for interface, async_read_some for async implementation, telnet:errors for error codes, RFC 854 for Telnet protocol
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
      requires asio::mutable_buffer_sequence<MutableBufferSequence>
    std::size_t socket<NLS, PC>::read_some(MutableBufferSequence buffers) {
        return sync_await(async_read_some(std::forward<MutableBufferSequence>(buffers), asio::use_awaitable));
    } //socket::read_some(MutableBufferSequence)

    /**
     * @brief Synchronously writes some data with Telnet-specific IAC escaping.
     * @tparam ConstBufferSequence The type of constant buffer sequence to write.
     * @param data The data to write (string or binary).
     * @return The number of bytes written.
     * @remark Wraps async_write_some using sync_await, propagating exceptions.
     * @see telnet-socket.cppm for interface, async_write_some for async implementation, telnet:errors for error codes, RFC 854 for IAC escaping
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
      requires asio::const_buffer_sequence<ConstBufferSequence>
    std::size_t socket<NLS, PC>::write_some(const ConstBufferSequence& data) {
        return sync_await(async_write_some(std::forward<ConstBufferSequence>(data), asio::use_awaitable));
    } //socket::write_some(const ConstBufferSequence&)

    /**
     * @brief Synchronously writes a Telnet command.
     * @param cmd The Telnet command to send.
     * @return The number of bytes written.
     * @remark Wraps async_write_command using sync_await, propagating exceptions.
     * @see telnet-socket.cppm for interface, async_write_command for async implementation, telnet:types for TelnetCommand, telnet:errors for error codes, RFC 854 for command structure
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    std::size_t socket<NLS, PC>::write_command(TelnetCommand cmd) {
        return sync_await(async_write_command(cmd, asio::use_awaitable));
    } //socket::write_command(TelnetCommand)

    /**
     * @brief Synchronously writes a Telnet negotiation command with an option.
     * @param cmd The Telnet command to send.
     * @param opt The option::id_num to send.
     * @return The number of bytes written.
     * @remark Wraps async_write_negotiation using sync_await, propagating exceptions.
     * @see telnet-socket.cppm for interface, async_write_negotiation for async implementation, telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for invalid_negotiation, RFC 855 for negotiation
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    std::size_t socket<NLS, PC>::write_negotiation(TelnetCommand cmd, option::id_num opt) {
        return sync_await(async_write_negotiation(cmd, opt, asio::use_awaitable));
    } //socket::write_negotiation(TelnetCommand, option::id_num)

    /**
     * @brief Synchronously writes a Telnet subnegotiation command.
     * @param opt The option::id_num for the subnegotiation.
     * @param subnegotiation_buffer The data buffer for the subnegotiation.
     * @return The number of bytes written.
     * @remark Wraps async_write_subnegotiation using sync_await, propagating exceptions.
     * @note Subnegotiation overflow is handled by ProtocolFSM::process_byte for incoming subnegotiations, not outgoing writes.
     * @see telnet-socket.cppm for interface, async_write_subnegotiation for async implementation, telnet:options for option::id_num, telnet:errors for error codes, telnet-protocol_fsm.cppm for ProtocolFSM, RFC 855 for subnegotiation
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    std::size_t socket<NLS, PC>::write_subnegotiation(option::id_num opt, const std::vector<std::uint8_t>& subnegotiation_buffer) {
        return sync_await(async_write_subnegotiation(opt, subnegotiation_buffer, asio::use_awaitable));
    } //socket::write_subnegotiation(option::id_num, const std::vector<std::uint8_t>&)

    /**
     * @remark Synchronous noexcept wrappers call synchronous throwing counterparts and convert exceptions to error codes.
     */
    /**
     * @brief Synchronously reads some data with Telnet processing, returning errors via error_code.
     * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
     * @param buffers The mutable buffer sequence to read into.
     * @param ec The error code to set on failure.
     * @return The number of bytes read, or 0 on error.
     * @remark Wraps throwing read_some, converting exceptions to error codes (system_error, not_enough_memory, internal_error).
     * @see telnet-socket.cppm for interface, read_some for throwing version, telnet:errors for error codes, RFC 854 for Telnet protocol
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
      requires asio::mutable_buffer_sequence<MutableBufferSequence>
    std::size_t socket<NLS, PC>::read_some(MutableBufferSequence buffers, std::error_code& ec) noexcept {
        try {
            return read_some(std::forward<MutableBufferSequence>(buffers));
        }
        catch (const std::system_error& e) {
            ec = e.code();
            return 0;
        }
        catch (const std::bad_alloc& e) {
            ec = std::make_error_code(std::errc::not_enough_memory);
            return 0;
        }
        catch (...) {
            ec = std::make_error_code(error::internal_error);
            return 0;
        }
    } //socket::read_some(MutableBufferSequence, std::error_code&) noexcept

    /**
     * @brief Synchronously writes some data with Telnet-specific IAC escaping, returning errors via error_code.
     * @tparam ConstBufferSequence The type of constant buffer sequence to write.
     * @param data The data to write (string or binary).
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps throwing write_some, converting exceptions to error codes (system_error, not_enough_memory, internal_error).
     * @see telnet-socket.cppm for interface, write_some for throwing version, telnet:errors for error codes, RFC 854 for IAC escaping
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
      requires asio::const_buffer_sequence<ConstBufferSequence>
    std::size_t socket<NLS, PC>::write_some(const ConstBufferSequence& data, std::error_code& ec) noexcept {
        try {
            return write_some(std::forward<ConstBufferSequence>(data));
        }
        catch (const std::system_error& e) {
            ec = e.code();
            return 0;
        }
        catch (const std::bad_alloc& e) {
            ec = std::make_error_code(std::errc::not_enough_memory);
            return 0;
        }
        catch (...) {
            ec = std::make_error_code(error::internal_error);
            return 0;
        }
    } //socket::write_some(const ConstBufferSequence&, std::error_code&) noexcept

    /**
     * @brief Synchronously writes a Telnet command, returning errors via error_code.
     * @param cmd The Telnet command to send.
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps throwing write_command, converting exceptions to error codes (system_error, not_enough_memory, internal_error).
     * @see telnet-socket.cppm for interface, write_command for throwing version, telnet:types for TelnetCommand, telnet:errors for error codes, RFC 854 for command structure
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    std::size_t socket<NLS, PC>::write_command(TelnetCommand cmd, std::error_code& ec) noexcept {
        try {
            return write_command(cmd);
        }
        catch (const std::system_error& e) {
            ec = e.code();
            return 0;
        }
        catch (const std::bad_alloc& e) {
            ec = std::make_error_code(std::errc::not_enough_memory);
            return 0;
        }
        catch (...) {
            ec = std::make_error_code(error::internal_error);
            return 0;
        }
    } //socket::write_command(TelnetCommand, std::error_code&) noexcept

    /**
     * @brief Synchronously writes a Telnet negotiation command with an option, returning errors via error_code.
     * @param cmd The Telnet command to send.
     * @param opt The option::id_num to send.
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps throwing write_negotiation, converting exceptions to error codes (system_error, not_enough_memory, internal_error).
     * @see telnet-socket.cppm for interface, write_negotiation for throwing version, telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for invalid_negotiation, RFC 855 for negotiation
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    std::size_t socket<NLS, PC>::write_negotiation(TelnetCommand cmd, option::id_num opt, std::error_code& ec) noexcept {
        try {
            return write_negotiation(cmd, opt);
        }
        catch (const std::system_error& e) {
            ec = e.code();
            return 0;
        }
        catch (const std::bad_alloc& e) {
            ec = std::make_error_code(std::errc::not_enough_memory);
            return 0;
        }
        catch (...) {
            ec = std::make_error_code(error::internal_error);
            return 0;
        }
    } //socket::write_negotiation(TelnetCommand, option::id_num, std::error_code&) noexcept

    /**
     * @brief Synchronously writes a Telnet subnegotiation command, returning errors via error_code.
     * @param opt The option::id_num for the subnegotiation.
     * @param subnegotiation_buffer The data buffer for the subnegotiation.
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps throwing write_subnegotiation, converting exceptions to error codes (system_error, not_enough_memory, internal_error).
     * @note Subnegotiation overflow is handled by ProtocolFSM::process_byte for incoming subnegotiations, not outgoing writes.
     * @see telnet-socket.cppm for interface, write_subnegotiation for throwing version, telnet:options for option::id_num, telnet:errors for error codes, telnet-protocol_fsm.cppm for ProtocolFSM, RFC 855 for subnegotiation
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    std::size_t socket<NLS, PC>::write_subnegotiation(option::id_num opt, const std::vector<std::uint8_t>& subnegotiation_buffer, std::error_code& ec) noexcept {
        try {
            return write_subnegotiation(opt, subnegotiation_buffer);
        }
        catch (const std::system_error& e) {
            ec = e.code();
            return 0;
        }
        catch (const std::bad_alloc& e) {
            ec = std::make_error_code(std::errc::not_enough_memory);
            return 0;
        }
        catch (...) {
            ec = std::make_error_code(error::internal_error);
            return 0;
        }
    } //socket::write_subnegotiation(option::id_num, const std::vector<std::uint8_t>&, std::error_code&) noexcept
} //namespace telnet