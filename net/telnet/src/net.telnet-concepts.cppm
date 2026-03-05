// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
/**
 * @file net.telnet-concepts.cppm
 * @version 0.5.8
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
 * @brief Module partition defining C++20 `concept`s for Telnet module type constraints.
 * @remark Defines concepts to constrain lower-layer stream/socket types for `telnet::stream`, ensuring compatibility with Boost.Asio stream socket requirements.
 * @remark Defines concept to constrain `ProtocolFSM` configurations.
 * @see RFC 854 for Telnet protocol requirements, `asio::ip::tcp::socket` for stream/socket interfaces, `:stream` for `telnet::stream` usage.
 *
 * @see `asio::ip::tcp::socket`, `asio::ssl::stream`, `:stream` for `telnet::stream`
 * @todo Future Development: Add concepts for TLS-specific stream requirements (e.g., for `boost::asio::ssl::stream` handshake methods).
 */

module; //Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio.hpp>

//Module partition interface unit
export module net.telnet:concepts;

import net.asio_concepts; //For asio_concepts namespace concept definitions

import std; //NOLINT For std::error_code, std::size_t, std::same_as, std::convertible_to

import :types;   ///< @see "net.telnet-types.cppm" for telnet::command
import :options; ///< @see "net.telnet-options.cppm" for telnet::option

export namespace net::telnet::concepts {
    //Forward declaration referenced in a following concept definition.
    template<typename ConfigT>
    class protocol_fsm;

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

    /**
     * @concept ProtocolFSMConfig
     * @brief Constraint on configuration types for `ProtocolFSM`.
     * @tparam T Configuration type
     * @remark Ensures `T` provides required types and operations for `ProtocolFSM` initialization and behavior.
     * @see `:protocol_fsm` for `ProtocolFSM`, `:protocol_config` for `DefaultProtocolFSMConfig`, RFC 854, RFC 855, RFC 1143
     */
    template<typename T>
    concept ProtocolFSMConfig =
        requires(
            T& config,
            telnet::command cmd,
            option full_opt,
            option::id_num opt,
            std::error_code& ec_out,
            std::error_code ec,
            byte_t byte,
            std::string msg
        ) {
            typename T::unknown_option_handler_type;
            requires std::
                convertible_to<typename T::unknown_option_handler_type, typename protocol_fsm<T>::unknown_option_handler_type>;
            typename T::error_logger_type;
            requires std::convertible_to<typename T::error_logger_type, typename protocol_fsm<T>::error_logger_type>;
            { T::initialize() } -> std::same_as<void>;
            { T::set_unknown_option_handler(std::declval<typename T::unknown_option_handler_type>()) } -> std::same_as<void>;
            { T::set_error_logger(std::declval<typename T::error_logger_type>()) } -> std::same_as<void>;
            {
                T::get_unknown_option_handler()
            } -> std::convertible_to<const typename protocol_fsm<T>::unknown_option_handler_type&>;
            { T::log_error(ec, msg) } -> std::same_as<void>;
            { T::registered_options.get(opt) } -> std::convertible_to<std::optional<option>>;
            { T::registered_options.has(opt) } -> std::same_as<bool>;
            { T::registered_options.upsert(opt) } -> std::convertible_to<const option&>;
            { T::registered_options.upsert(full_opt, ec_out) } -> std::same_as<void>;
            { T::registered_options.upsert(full_opt) } -> std::convertible_to<const option&>;
            { T::get_ayt_response() } -> std::same_as<std::string_view>;
            { T::set_ayt_response(msg) } -> std::same_as<void>;
        }; //concept ProtocolFSMConfig
} //namespace net::telnet::concepts

namespace net::telnet::concepts {
    /**
     * @concept Awaiter
     * @brief Concept that checks whether a type satisfies the core requirements of a coroutine awaiter.
     *
     * A type models `Awaiter` if it provides the three mandatory member functions of an awaiter:
     *   - `await_ready()` returning a contextually convertible-to-bool value,
     *   - `await_suspend(coroutine_handle<>)` (may return `void`, `bool`, or `coroutine_handle<>`),
     *   - `await_resume()` returning the result of the co_await expression.
     *
     * Both const/lvalue and rvalue overloads are required.
     *
     * @see [expr.await] in the C++ standard
     * @see IntrinsicallyAwaitable
     */
    export template<typename T>
    concept Awaiter = requires(T& awaiter, std::coroutine_handle<> handle) {
                          { awaiter.await_ready() } -> std::convertible_to<bool>;
                          awaiter.await_suspend(handle);
                          awaiter.await_resume();

                          { std::move(awaiter).await_ready() } -> std::convertible_to<bool>;
                          std::move(awaiter).await_suspend(handle);
                          std::move(awaiter).await_resume();
                      };

    /**
     * @internal
     * @concept AwaitableByMember
     * @brief Concept that checks whether a type provides a member `operator co_await()`.
     *
     * If this concept is satisfied, then `co_await expr` will call `expr.operator co_await()` and use the returned awaiter object.
     *
     * @see Awaiter
     * @see IntrinsicallyAwaitable
     */
    template<typename T>
    concept AwaitableByMember = requires(T&& awaitable) {
                                    { std::forward<T>(awaitable).operator co_await() } -> Awaiter;
                                };

    /**
     * @internal
     * @concept AwaitableByADL
     * @brief Concept that checks whether a free function `operator co_await` can be found via ADL.
     *
     * If this concept is satisfied, then `co_await expr` can perform ADL lookup for `operator co_await(expr)` and use the returned awaiter object.
     *
     * @see Awaiter
     * @see IntrinsicallyAwaitable
     */
    template<typename T>
    concept AwaitableByADL = requires(T&& awaitable) {
                                 { operator co_await(std::forward<T>(awaitable)) } -> Awaiter;
                             };

    /**
     * @internal
     * @concept IntrinsicallyAwaitable
     * @brief Composite concept that checks whether a type can intrinsically be used in a co_await expression.
     *
     * A type `T` satisfies `IntrinsicallyAwaitable` if **any** of the following is true:
     *   - it has a member `operator co_await()` that returns an awaiter (`AwaitableByMember`),
     *   - it has an ADL-found `operator co_await(T)` that returns an awaiter (`AwaitableByADL`).
     *   - it models `Awaiter` directly (many awaitables are awaiters themselves),
     *
     * This is the concept to use when constraining a template parameter that is expected to appear after `co_await`.
     *
     * @see Awaiter
     * @see AwaitableByMember
     * @see AwaitableByADL
     * @see [expr.await] in the C++ standard
     */
    template<typename T>
    concept IntrinsicallyAwaitable = AwaitableByMember<T> || AwaitableByADL<T> || Awaiter<T>;
    
    /**
     * @internal
     * @concept ContextuallySelfAwaitableTask
     * @brief Concept that checks if a coroutine task type's promise type implements await_transform with the task type as the parameter.
     *
     * If this concept is satisfied then for a type `T`, `T::promise_type` has member `await_transform(T)` and a coroutine returning type `T` can `co_await` an object of type `T`.
     *
     * @see Awaitable
     * @see IntrinsicallyAwaitable
     */
    template<typename T>
    concept ContextuallySelfAwaitableTask = requires(typename T::promise_type& promise, T& target) {
                                                { promise await_transform(target) } -> IntrinsicallyAwaitable;
                                            };

    /**
     * @concept Awaitable
     * @brief Composite concept that checks whether a type can be used in a co_await expression.
     *
     * @see IntrinsicallyAwaitable
     * @see ContextuallySelfAwaitableTask
     */
    export template<typename T>
    concept Awaitable = IntrinsicallyAwaitable<T> || ContextuallySelfAwaitableTask<T>;
} //namespace net::telnet::concepts
