/**
 * @file net.telnet-concepts.cppm
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Module partition defining C++20 `concept`s for Telnet module type constraints.
 * @remark Defines concepts to constrain lower-layer stream/socket types for `telnet::stream`, ensuring compatibility with Boost.Asio stream socket requirements.
 * @remark Defines concept to constrain `ProtocolFSM` configurations.
 * @see RFC 854 for Telnet protocol requirements, `asio::ip::tcp::socket` for stream/socket interfaces, `:stream` for `telnet::stream` usage.
 * 
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see `asio::ip::tcp::socket`, `asio::ssl::stream`, `:stream` for `telnet::stream`
 * @todo Future Development: Add concepts for TLS-specific stream requirements (e.g., for `boost::asio::ssl::stream` handshake methods).
 */

module; // Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <asio.hpp>

// Module partition interface unit
export module net.telnet:concepts;

import net.asio_concepts; // For asio_concepts namespace concept definitions

import std; // For std::error_code, std::size_t, std::same_as, std::convertible_to

//namespace asio = boost::asio;

export namespace net::telnet::concepts {
    /**
     * @concept MutableBufferSequence
     * @brief Alias for a type modeling Boost.Asio's "MutableBufferSequence" requirement.
     * @tparam T The type to check.
     * @remark Delegates to `asio_concepts::AsioMutableBufferSequence`.
     * @see `asio::mutable_buffer`, `asio::is_mutable_buffer_sequence`, `asio_concepts::AsioMutableBufferSequence`
     */
    template<typename T>
    concept MutableBufferSequence = asio_concepts::AsioMutableBufferSequence<T>;
    
    /**
     * @concept ConstBufferSequence
     * @brief Alias for a type modeling Boost.Asio's "ConstBufferSequence" requirement.
     * @tparam T The type to check.
     * @remark Delegates to `asio_concepts::AsioConstBufferSequence`.
     * @see `asio::const_buffer`, `asio::is_const_buffer_sequence`, `asio_concepts::AsioConstBufferSequence`
     */
    template<typename T>
    concept ConstBufferSequence = asio_concepts::AsioConstBufferSequence<T>;
    
    /**
     * @concept ReadToken
     * @brief Alias for a completion token usable with asynchronous read operations.
     * @tparam T The type to check.
     * @remark Delegates to `asio_concepts::AsioReadToken`.
     * @see `asio::use_awaitable`, `asio::completion_token_for`, `asio_concepts::AsioReadToken`
     */
    template<typename T>
    concept ReadToken = asio_concepts::AsioReadToken<T>;
    
    /**
     * @concept WriteToken
     * @brief Alias for a completion token usable with asynchronous write operations.
     * @tparam T The type to check.
     * @remark Delegates to `asio_concepts::AsioWriteToken`.
     * @see `asio::use_awaitable`, `asio::completion_token_for`, `asio_concepts::AsioWriteToken`
     */
    template<typename T>
    concept WriteToken = asio_concepts::AsioWriteToken<T>;
    
    /**
     * @concept LayerableSocketStream
     * @brief Layered stream/socket types compatible with `telnet::stream` operations.
     * @tparam T The type to check.
     * @remark Combines Asio stream protocols, executor, layering, endpoint, and Telnet-specific requirements.
     * @remark Ensures compatibility with `boost::asio::ip::tcp::socket` and layered streams like `boost::asio::ssl::stream`.
     * @see RFC 854 for Telnet protocol, `boost::asio::ip::tcp::socket`, `:stream` for `telnet::stream`
     */
    template<typename T>
    concept LayerableSocketStream = asio_concepts::AsioLayerableStreamSocket<T>;
    
    
    //Forward declaration referenced in the following concept definition.
    template<typename ConfigT> class ProtocolFSM;
    
    /**
     * @concept ProtocolFSMConfig
     * @brief Constraint on configuration types for `ProtocolFSM`.
     * @tparam T Configuration type
     * @remark Ensures `T` provides required types and operations for `ProtocolFSM` initialization and behavior.
     * @see `:protocol_fsm` for `ProtocolFSM`, `:protocol_config` for `DefaultProtocolFSMConfig`, RFC 854, RFC 855, RFC 1143
     */
    template<typename T>
    concept ProtocolFSMConfig =
        requires(T& config, TelnetCommand cmd, option full_opt, option::id_num opt, std::error_code& ec_out, std::error_code ec, byte_t byte, std::string msg) {
            typename T::UnknownOptionHandler;
            requires std::convertible_to<typename T::UnknownOptionHandler, typename ProtocolFSM<T>::UnknownOptionHandler>;
            typename T::ErrorLogger;
            requires std::convertible_to<typename T::ErrorLogger, typename ProtocolFSM<T>::ErrorLogger>;
            { T::initialize() } -> std::same_as<void>;
            { T::set_unknown_option_handler(std::declval<typename T::UnknownOptionHandler>()) } -> std::same_as<void>;
            { T::set_error_logger(std::declval<typename T::ErrorLogger>()) } -> std::same_as<void>;
            { T::get_unknown_option_handler() } -> std::convertible_to<const typename ProtocolFSM<T>::UnknownOptionHandler&>;
            { T::log_error(ec, msg) } -> std::same_as<void>;
            { T::registered_options.get(opt) } -> std::convertible_to<std::optional<option>>;
            { T::registered_options.has(opt) } -> std::same_as<bool>;
            { T::registered_options.upsert(opt) } -> std::convertible_to<const option&>;
            { T::registered_options.upsert(full_opt, ec_out) } -> std::same_as<void>;
            { T::registered_options.upsert(full_opt) } -> std::convertible_to<const option&>;
            { T::get_ayt_response() } -> std::same_as<std::string_view>;
            { T::set_ayt_response(msg) } -> std::same_as<void>;
        }; // concept ProtocolFSMConfig
} // export namespace net::telnet::concepts
