/**
 * @file telnet-socket-sync-impl.cpp
 * @version 0.4.0
 * @release_date October 3, 2025
 *
 * @brief Implementation of synchronous Telnet socket operations.
 * @remark Contains `read_some`, `write_some`, `write_raw`, `write_command`, `write_negotiation`, and `write_subnegotiation`, wrapping asynchronous counterparts.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @note Synchronous I/O operations incur overhead from `sync_await`, which creates a new `io_context` and thread per call, and scale poorly compared to asynchronous methods. However, this overhead is minimal compared to blocking network I/O latency.
 * @see telnet-socket.cppm for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, :types for `TelnetCommand`, :options for `option`, :errors for error codes, :protocol_fsm for `ProtocolFSM`
 */
module; // Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

// Module partition implementation unit
module telnet:socket;

import std; // For std::size_t, std::system_error

import :types;        ///< @see telnet-types.cppm for `TelnetCommand`
import :errors;       ///< @see telnet-errors.cppm for `telnet::error` codes
import :options;      ///< @see telnet-options.cppm for `option`
import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm for `ProtocolFSM`

import :socket;       ///< @see telnet-socket.cppm for socket partition interface

namespace telnet {
    //=========================================================================================================
    // Synchronous throwing wrappers use `sync_await` to execute their asynchronous counterparts.
    //=========================================================================================================
    /**
     * @internal
     * Wraps awaitable `async_read_some` in `sync_await` with perfect forwarding of the buffer sequence.
     * @see `async_read_some` in `telnet-socket-async-impl.cpp`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
      requires asio::mutable_buffer_sequence<MutableBufferSequence>
    std::size_t socket<NLS, PC>::read_some(MutableBufferSequence&& buffers) {
        return sync_await(async_read_some(std::forward<MutableBufferSequence>(buffers), asio::use_awaitable));
    } // socket::read_some(MutableBufferSequence&&)

    /**
     * @internal
     * Wraps awaitable `async_write_some` in `sync_await`, forwarding the buffer sequence.
     * @see `async_write_some` in `telnet-socket-async-impl.cpp`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
      requires asio::const_buffer_sequence<ConstBufferSequence>
    std::size_t socket<NLS, PC>::write_some(const ConstBufferSequence& data) {
        return sync_await(async_write_some(data, asio::use_awaitable));
    } // socket::write_some(const ConstBufferSequence&)

    /**
     * @internal
     * Wraps awaitable `async_write_raw` in `sync_await`, forwarding the buffer sequence.
     * @see `async_write_raw` in `telnet-socket-async-impl.cpp`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
      requires asio::const_buffer_sequence<ConstBufferSequence>
    std::size_t socket<NLS, PC>::write_raw(const ConstBufferSequence& data) {
        return sync_await(async_write_raw(data, asio::use_awaitable));
    } // socket::write_raw(const ConstBufferSequence&)
    
    /**
     * @internal
     * Wraps awaitable `async_write_command` in `sync_await`, forwarding the command.
     * @see `async_write_command` in `telnet-socket-async-impl.cpp`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    std::size_t socket<NLS, PC>::write_command(TelnetCommand cmd) {
        return sync_await(async_write_command(cmd, asio::use_awaitable));
    } // socket::write_command(TelnetCommand)

    /**
     * @internal
     * Wraps awaitable `async_write_negotiation` in `sync_await`, forwarding the command and option.
     * @see `async_write_negotiation` in `telnet-socket-async-impl.cpp`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    std::size_t socket<NLS, PC>::write_negotiation(TelnetCommand cmd, option::id_num opt) {
        return sync_await(async_write_negotiation(cmd, opt, asio::use_awaitable));
    } // socket::write_negotiation(TelnetCommand, option::id_num)

    /**
     * @internal
     * Wraps awaitable `async_write_subnegotiation` in `sync_await`, forwarding the option and buffer.
     * @see `async_write_subnegotiation` in `telnet-socket-async-impl.cpp`.
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    std::size_t socket<NLS, PC>::write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer) {
        return sync_await(async_write_subnegotiation(opt, subnegotiation_buffer, asio::use_awaitable));
    } // socket::write_subnegotiation(option, const std::vector<byte_t>&)

    //=========================================================================================================
    // Synchronous `noexcept` wrappers call their throwing counterparts and convert exceptions to error codes.
    //=========================================================================================================
    /**
     * @internal
     * Calls the throwing `read_some`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `read_some` for throwing version, `async_read_some` in `telnet-socket-async-impl.cpp` for async implementation, `telnet-socket.cppm` for interface
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
      requires asio::mutable_buffer_sequence<MutableBufferSequence>
    std::size_t socket<NLS, PC>::read_some(MutableBufferSequence&& buffers, std::error_code& ec) noexcept {
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
    } // socket::read_some(MutableBufferSequence&&, std::error_code&) noexcept

    /**
     * @internal
     * Calls the throwing `write_some`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `write_some` for throwing version, `async_write_some` in `telnet-socket-async-impl.cpp` for async implementation, `telnet-socket.cppm` for interface
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
      requires asio::const_buffer_sequence<ConstBufferSequence>
    std::size_t socket<NLS, PC>::write_some(const ConstBufferSequence& data, std::error_code& ec) noexcept {
        try {
            return write_some(data);
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
    } // socket::write_some(const ConstBufferSequence&, std::error_code&) noexcept

    /**
     * @internal
     * Calls the throwing `write_raw`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `write_raw` for throwing version, `async_write_raw` in `telnet-socket-async-impl.cpp` for async implementation, `telnet-socket.cppm` for interface
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
      requires asio::const_buffer_sequence<ConstBufferSequence>
    std::size_t socket<NLS, PC>::write_raw(const ConstBufferSequence& data, std::error_code& ec) noexcept {
        try {
            return write_raw(data);
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
    } // socket::write_raw(const ConstBufferSequence&, std::error_code&) noexcept
    
    /**
     * @internal
     * Calls the throwing `write_command`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `write_command` for throwing version, `async_write_command` in `telnet-socket-async-impl.cpp` for async implementation, `telnet-socket.cppm` for interface
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
    } // socket::write_command(TelnetCommand, std::error_code&) noexcept

    /**
     * @internal
     * Calls the throwing `write_negotiation`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `write_negotiation` for throwing version, `async_write_negotiation` in `telnet-socket-async-impl.cpp` for async implementation, `telnet-socket.cppm` for interface
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
    } // socket::write_negotiation(TelnetCommand, option::id_num, std::error_code&) noexcept

    /**
     * @internal
     * Calls the throwing `write_subnegotiation`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `write_subnegotiation` for throwing version, `async_write_subnegotiation` in `telnet-socket-async-impl.cpp` for async implementation, `telnet-socket.cppm` for interface
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    std::size_t socket<NLS, PC>::write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer, std::error_code& ec) noexcept {
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
    } // socket::write_subnegotiation(option, const std::vector<byte_t>&, std::error_code&) noexcept
} // namespace telnet