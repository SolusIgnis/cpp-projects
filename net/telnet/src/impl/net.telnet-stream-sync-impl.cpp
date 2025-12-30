/**
 * @file net.telnet-stream-sync-impl.cpp
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Implementation of synchronous Telnet stream operations.
 * @remark Contains `read_some`, `write_some`, `write_raw`, `write_command`, `write_negotiation`, `write_subnegotiation`, `send_synch`, `request_option`, and `disable_option`, wrapping asynchronous counterparts.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @note Synchronous I/O operations incur overhead from `sync_await`, which creates a new `io_context` and thread per call, and scale poorly compared to asynchronous methods. However, this overhead is minimal compared to blocking network I/O latency.
 * @see "net.telnet-stream.cppm" for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, `:types` for `telnet::command`, `:options` for `option`, `:errors` for error codes, `:protocol_fsm` for `ProtocolFSM`
 */

module; //Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio.hpp>

//Module implementation unit
module net.telnet;

import std; //For std::size_t, std::system_error

import :types;        ///< @see "net.telnet-types.cppm" for `telnet::command`
import :errors;       ///< @see "net.telnet-errors.cppm" for `telnet::error` and `telnet::processing_signal` codes
import :concepts;     ///< @see "net.telnet-concepts.cppm" for `telnet::concepts::LayerableSocketStream`
import :options;      ///< @see "net.telnet-options.cppm" for `option`
import :protocol_fsm; ///< @see "net.telnet-protocol_fsm.cppm" for `ProtocolFSM`

import :stream;       ///< @see "net.telnet-stream.cppm" for the partition being implemented.

//namespace asio = boost::asio;

namespace net::telnet {
    //=========================================================================================================
    //Synchronous throwing wrappers use `sync_await` to execute their asynchronous counterparts.
    //=========================================================================================================

    /**
     * @internal
     * Wraps awaitable `async_request_option` in `sync_await`, forwarding the option and direction.
     * @see `async_request_option` in "net.telnet-stream-async-impl.cpp".
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    std::size_t stream<NLS, PC>::request_option(option::id_num opt, negotiation_direction direction) {
        auto [ec, bytes] = sync_await(async_request_option(opt, direction, asio::use_awaitable));
        if (ec) {
            throw std::system_error(ec);
        }
        return bytes;
    } //stream::request_option(option::id_num, negotiation_direction)

    /**
     * @internal
     * Wraps awaitable `async_disable_option` in `sync_await`, forwarding the option and direction.
     * @see `async_disable_option` in "net.telnet-stream-async-impl.cpp".
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    std::size_t stream<NLS, PC>::disable_option(option::id_num opt, negotiation_direction direction) {
        auto [ec, bytes] = sync_await(async_disable_option(opt, direction, asio::use_awaitable));
        if (ec) {
            throw std::system_error(ec);
        }
        return bytes;
    } //stream::disable_option(option::id_num, negotiation_direction)

    /**
     * @internal
     * Wraps awaitable `async_read_some` in `sync_await` with perfect forwarding of the buffer sequence.
     * @see `async_read_some` in "net.telnet-stream-async-impl.cpp".
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC, MutableBufferSequence MBufSeq>
    std::size_t stream<NLS, PC>::read_some(MBufSeq&& buffers) {
        return sync_await(async_read_some(std::forward<MBufSeq>(buffers), asio::use_awaitable));
    } //stream::read_some(MBufSeq&&)

    /**
     * @internal
     * Wraps awaitable `async_write_some` in `sync_await`, forwarding the buffer sequence.
     * @see `async_write_some` in "net.telnet-stream-async-impl.cpp".
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC, ConstBufferSequence CBufSeq>
    std::size_t stream<NLS, PC>::write_some(const CBufSeq& data) {
        return sync_await(async_write_some(data, asio::use_awaitable));
    } //stream::write_some(const CBufSeq&)

    /**
     * @internal
     * Wraps awaitable `async_write_raw` in `sync_await`, forwarding the buffer sequence.
     * @see `async_write_raw` in "net.telnet-stream-async-impl.cpp".
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC, ConstBufferSequence CBufSeq>
    std::size_t stream<NLS, PC>::write_raw(const CBufSeq& data) {
        return sync_await(async_write_raw(data, asio::use_awaitable));
    } //stream::write_raw(const CBufSeq&)
    
    /**
     * @internal
     * Wraps awaitable `async_write_command` in `sync_await`, forwarding the command.
     * @see `async_write_command` in "net.telnet-stream-async-impl.cpp".
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    std::size_t stream<NLS, PC>::write_command(telnet::command cmd) {
        return sync_await(async_write_command(cmd, asio::use_awaitable));
    } //stream::write_command(telnet::command)

    /**
     * @internal
     * Wraps awaitable `async_write_subnegotiation` in `sync_await`, forwarding the option and buffer.
     * @see `async_write_subnegotiation` in "net.telnet-stream-async-impl.cpp".
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    std::size_t stream<NLS, PC>::write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer) {
        return sync_await(async_write_subnegotiation(opt, subnegotiation_buffer, asio::use_awaitable));
    } //stream::write_subnegotiation(option, const std::vector<byte_t>&)

    /**
     * @internal
     * Wraps awaitable `async_send_synch` in `sync_await`, forwarding the command and option.
     * @see `async_send_synch` in "net.telnet-stream-async-impl.cpp".
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    std::size_t stream<NLS, PC>::send_synch() {
        return sync_await(async_send_synch(asio::use_awaitable));
    } //stream::send_synch()

    //=========================================================================================================
    //Synchronous `noexcept` wrappers call their throwing counterparts and convert exceptions to error codes.
    //=========================================================================================================

    /**
     * @internal
     * Calls the throwing `request_option`, catching exceptions to set `ec` to `std::system_error`’s code or `telnet::error::internal_error`.
     * @see `request_option` for throwing version, `async_request_option` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    std::size_t stream<NLS, PC>::request_option(option::id_num opt, negotiation_direction direction, std::error_code& ec) noexcept {
        try {
            return request_option(opt, direction);
        }
        catch (const std::system_error& e) {
            ec = e.code();
            return 0;
        }
        catch (...) {
            ec = std::make_error_code(error::internal_error);
            return 0;
        }
    } //stream::request_option(option::id_num, negotiation_direction, std::error_code&) noexcept

    /**
     * @internal
     * Calls the throwing `disable_option`, catching exceptions to set `ec` to `std::system_error`’s code or `telnet::error::internal_error`.
     * @see `disable_option` for throwing version, `async_disable_option` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    std::size_t stream<NLS, PC>::disable_option(option::id_num opt, negotiation_direction direction, std::error_code& ec) noexcept {
        try {
            return disable_option(opt, direction);
        }
        catch (const std::system_error& e) {
            ec = e.code();
            return 0;
        }
        catch (...) {
            ec = std::make_error_code(error::internal_error);
            return 0;
        }
    } //stream::disable_option(option::id_num, negotiation_direction, std::error_code&) noexcept

    /**
     * @internal
     * Calls the throwing `read_some`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `read_some` for throwing version, `async_read_some` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
      requires asio::mutable_buffer_sequence<MutableBufferSequence>
    std::size_t stream<NLS, PC>::read_some(MutableBufferSequence&& buffers, std::error_code& ec) noexcept {
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
    } //stream::read_some(MutableBufferSequence&&, std::error_code&) noexcept

    /**
     * @internal
     * Calls the throwing `write_some`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `write_some` for throwing version, `async_write_some` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC, ConstBufferSequence CBufSeq>
      requires asio::const_buffer_sequence<ConstBufferSequence>
    std::size_t stream<NLS, PC>::write_some(const CBufSeq& data, std::error_code& ec) noexcept {
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
    } //stream::write_some(const CBufSeq&, std::error_code&) noexcept

    /**
     * @internal
     * Calls the throwing `write_raw`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `write_raw` for throwing version, `async_write_raw` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC, ConstBufferSequence CBufSeq>
      requires asio::const_buffer_sequence<ConstBufferSequence>
    std::size_t stream<NLS, PC>::write_raw(const CBufSeq& data, std::error_code& ec) noexcept {
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
    } //stream::write_raw(const CBufSeq&, std::error_code&) noexcept
    
    /**
     * @internal
     * Calls the throwing `write_command`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `write_command` for throwing version, `async_write_command` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    std::size_t stream<NLS, PC>::write_command(telnet::command cmd, std::error_code& ec) noexcept {
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
    } //stream::write_command(telnet::command, std::error_code&) noexcept

    /**
     * @internal
     * Calls the throwing `write_subnegotiation`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `write_subnegotiation` for throwing version, `async_write_subnegotiation` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    std::size_t stream<NLS, PC>::write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer, std::error_code& ec) noexcept {
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
    } //stream::write_subnegotiation(option, const std::vector<byte_t>&, std::error_code&) noexcept
    
    /**
     * @internal
     * Calls the throwing `send_synch`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.
     * @see `send_synch` for throwing version, `async_send_synch` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface
     */
    template<LayerableSocketStream NLS, ProtocolFSMConfig PC>
    std::size_t stream<NLS, PC>::send_synch(std::error_code& ec) noexcept {
        try {
            return send_synch();
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
    } //stream::send_synch(std::error_code&) noexcept
} //namespace net::telnet
