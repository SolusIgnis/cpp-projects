// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
/**
 * @module net.asio_concepts
 * @file net.asio_concepts.cppm
 * @version 0.1.2
 * @date February 25, 2026
 *
 * @copyright © 2025-2026 Jeremy Murphy and any Contributors
 * @par License: @parblock
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. @endparblock
 *
 * @brief Module defining reusable C++20 `concept`s for Asio I/O object requirements.
 * @remark Provides fine-grained, composable concepts for Asio stream, socket, and layered types.
 * @see `asio::ip::tcp::socket`, `asio::ssl::stream`, `telnet:concepts`
 */

module; // Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio.hpp>

// Primary module interface unit
export module net.asio_concepts;

import std; // For std::error_code, std::size_t, std::same_as, std::convertible_to

namespace net::asio_concepts {
    /**
     * @typedef asio_read_completion_signature
     * @brief Completion handler signature for Boost.Asio asynchronous read operations.
     * @remark Defines the signature `void(std::error_code, std::size_t)` for completion handlers.
     */
    using asio_read_completion_signature = void(std::error_code, std::size_t);

    /**
     * @typedef asio_write_completion_signature
     * @brief Completion handler signature for Boost.Asio asynchronous write operations.
     * @remark Defines the signature `void(std::error_code, std::size_t)` for completion handlers.
     */
    using asio_write_completion_signature = void(std::error_code, std::size_t);

    /**
     * @typedef asio_wait_completion_signature
     * @brief Completion handler signature for Boost.Asio asynchronous wait operations.
     * @remark Defines the signature `void(std::error_code)` for completion handlers.
     */
    using asio_wait_completion_signature = void(std::error_code);

    /**
     * @typedef asio_connect_completion_signature
     * @brief Completion handler signature for Boost.Asio asynchronous connect operations.
     * @remark Defines the signature `void(std::error_code)` for completion handlers.
     */
    using asio_connect_completion_signature = void(std::error_code);

    /**
     * @typedef asio_sample_completion_token
     * Canonical default completion token with general completion executor.
     * @warning This is used to test for a CompletionToken template parameter in some `concept`s.
     */
    using asio_sample_completion_token = asio::default_completion_token_t<asio::any_completion_executor>;
} // namespace net::asio_concepts

export namespace net::asio_concepts {
    inline namespace buffers {
        /**
         * @concept asio_mutable_buffer_sequence
         * @brief Concept for types modeling "MutableBufferSequence".
         * @tparam T The type to check.
         * @see `boost::asio::mutable_buffer`, `std::array<mutable_buffer, N>`
         */
        template<typename T>
        concept asio_mutable_buffer_sequence = asio::is_mutable_buffer_sequence<T>::value;

        /**
         * @concept asio_const_buffer_sequence
         * @brief Concept for types modeling "ConstBufferSequence".
         * @tparam T The type to check.
         * @see `boost::asio::const_buffer`, `std::span<const_buffer>`
         */
        template<typename T>
        concept asio_const_buffer_sequence = asio::is_const_buffer_sequence<T>::value;
    } //namespace buffers

    inline namespace tokens {
        /**
         * @concept asio_read_token
         * @brief Concept for types modeling a "CompletionToken" for read operations.
         * @tparam T The type to check.
         * @see `asio::use_awaitable`, `asio::detached`
         */
        template<typename T>
        concept asio_read_token = asio::completion_token_for<T, asio_read_completion_signature>;

        /**
         * @concept asio_write_token
         * @brief Concept for types modeling a "CompletionToken" for write operations.
         * @tparam T The type to check.
         * @see `asio::use_awaitable`, `asio::detached`
         */
        template<typename T>
        concept asio_write_token = asio::completion_token_for<T, asio_write_completion_signature>;

        /**
         * @concept asio_wait_token
         * @brief Concept for types modeling a "CompletionToken" for wait operations.
         * @tparam T The type to check.
         * @see `asio::use_awaitable`, `asio::detached`
         */
        template<typename T>
        concept asio_wait_token = asio::completion_token_for<T, asio_wait_completion_signature>;

        /**
         * @concept asio_connect_token
         * @brief Concept for types modeling a "CompletionToken" for connect operations.
         * @tparam T The type to check.
         * @see `asio::use_awaitable`, `asio::detached`
         */
        template<typename T>
        concept asio_connect_token = asio::completion_token_for<T, asio_connect_completion_signature>;
    } //namespace tokens

    inline namespace socket_options {
        /**
         * @concept boolean_socket_option
         * @brief Concept for types modeling boolean-valued socket options.
         * @tparam T The type to check.
         * @see `asio::socket_base::broadcast`, `reuse_address`
         */
        template<typename T>
        concept boolean_socket_option = std::default_initializable<T> && requires(T temp) {
                                                                           {
                                                                               std::as_const(temp).value()
                                                                           } -> std::convertible_to<bool>;
                                                                           requires std::constructible_from<T, bool>;
                                                                       };

        /**
         * @concept integral_socket_option
         * @brief Concept for types modeling integer-valued socket options.
         * @tparam T The type to check.
         * @see `asio::socket_base::receive_buffer_size`
         */
        template<typename T>
        concept integral_socket_option = std::default_initializable<T>
                                    && requires(T temp) {
                                           { std::as_const(temp).value() };
                                           requires (!(std::same_as<decltype(std::as_const(temp).value()), bool>));
                                           requires std::integral<decltype(std::as_const(temp).value())>;
                                           requires std::constructible_from<T, decltype(std::as_const(temp).value())>;
                                       };

        /**
         * @concept composite_socket_option
         * @brief Concept for types modeling composite-valued (boolean/integer pair) socket options.
         * @tparam T The type to check.
         * @see `boost::asio::socket_base::linger`
         */
        template<typename T>
        concept composite_socket_option = std::default_initializable<T>
                                     && requires(T temp) {
                                            { std::as_const(temp).enabled() } -> std::convertible_to<bool>;
                                            { std::as_const(temp).timeout() };
                                            requires std::integral<decltype(std::as_const(temp).timeout())>;
                                            requires std::constructible_from<T, bool, decltype(std::as_const(temp).timeout())>;
                                        };

        /**
         * @concept asio_addressible_socket_option
         * @brief Concept for types modeling address-constructed socket options.
         * @tparam T The type to check.
         * @see `asio::ip::multicast::join_group`
         */
        template<typename T>
        concept asio_addressible_socket_option = std::default_initializable<T> && std::constructible_from<T, asio::ip::address>;

        /**
         * @concept socket_option
         * @brief Concept for types modeling general socket options.
         * @tparam T The type to check.
         * @remark Allows any of the four basic option types.
         */
        template<typename T>
        concept socket_option = boolean_socket_option<T> || integral_socket_option<T> || composite_socket_option<T>
                            || asio_addressible_socket_option<T>;

        /**
         * @concept socket_option_provider
         * @brief Concept for types providing the standard set of socket options.
         * @tparam T The type to check.
         * @remark Requires all standard `socket_base` option types to satisfy their respective concepts.
         */
        template<typename T>
        concept socket_option_provider = requires {
                                           requires boolean_socket_option<typename T::broadcast>;
                                           requires boolean_socket_option<typename T::debug>;
                                           requires boolean_socket_option<typename T::do_not_route>;
                                           requires boolean_socket_option<typename T::enable_connection_aborted>;
                                           requires boolean_socket_option<typename T::keep_alive>;
                                           requires composite_socket_option<typename T::linger>;
                                           requires boolean_socket_option<typename T::out_of_band_inline>;
                                           requires integral_socket_option<typename T::receive_buffer_size>;
                                           requires integral_socket_option<typename T::receive_low_watermark>;
                                           requires boolean_socket_option<typename T::reuse_address>;
                                           requires integral_socket_option<typename T::send_buffer_size>;
                                           requires integral_socket_option<typename T::send_low_watermark>;
                                       };

        /**
         * @concept has_gettable_socket_option
         * @brief Concept for getting a specific socket option.
         * @tparam T The socket type.
         * @tparam Option The option type.
         * @remark Requires both error-code and non-throwing overloads.
         */
        template<typename T, typename Option>
        concept has_gettable_socket_option = requires(T& temp, Option& opt, std::error_code& ec_out) {
                                              { temp.get_option(opt) } -> std::same_as<void>;
                                              { temp.get_option(opt, ec_out) };
                                          };

        /**
         * @concept socket_option_getter
         * @brief Concept for types that support getting all standard socket options.
         * @tparam T The type to check.
         */
        template<typename T>
        concept socket_option_getter = has_gettable_socket_option<T, asio::socket_base::broadcast>
                                  && has_gettable_socket_option<T, asio::socket_base::debug>
                                  && has_gettable_socket_option<T, asio::socket_base::do_not_route>
                                  && has_gettable_socket_option<T, asio::socket_base::enable_connection_aborted>
                                  && has_gettable_socket_option<T, asio::socket_base::keep_alive>
                                  && has_gettable_socket_option<T, asio::socket_base::linger>
                                  && has_gettable_socket_option<T, asio::socket_base::out_of_band_inline>
                                  && has_gettable_socket_option<T, asio::socket_base::receive_buffer_size>
                                  && has_gettable_socket_option<T, asio::socket_base::receive_low_watermark>
                                  && has_gettable_socket_option<T, asio::socket_base::reuse_address>
                                  && has_gettable_socket_option<T, asio::socket_base::send_buffer_size>
                                  && has_gettable_socket_option<T, asio::socket_base::send_low_watermark>;

        /**
         * @concept has_unary_settable_socket_option
         * @brief Concept for setting a unary socket option.
         * @tparam T The socket type.
         * @tparam Option The option type.
         * @tparam Arg The argument type.
         */
        template<typename T, typename Option, typename Arg>
        concept has_unary_settable_socket_option = requires(T& temp, std::error_code& ec_out, Arg& arg) {
                                                   { temp.set_option(Option(arg)) } -> std::same_as<void>;
                                                   { temp.set_option(Option(arg), ec_out) };
                                               };

        /**
         * @concept has_binary_settable_socket_option
         * @brief Concept for setting a binary socket option.
         * @tparam T The socket type.
         * @tparam Option The option type.
         * @tparam Arg1, Arg2 Argument types.
         */
        template<typename T, typename Option, typename Arg1, typename Arg2>
        concept has_binary_settable_socket_option = requires(T& temp, std::error_code& ec_out, Arg1 arg1, Arg2 arg2) {
                                                    { temp.set_option(Option(arg1, arg2)) } -> std::same_as<void>;
                                                    { temp.set_option(Option(arg1, arg2), ec_out) };
                                                };

        /**
         * @concept socket_option_setter
         * @brief Concept for types that support setting all standard socket options.
         * @tparam T The type to check.
         */
        template<typename T>
        concept socket_option_setter = has_unary_settable_socket_option<T, asio::socket_base::broadcast, bool>
                                  && has_unary_settable_socket_option<T, asio::socket_base::debug, bool>
                                  && has_unary_settable_socket_option<T, asio::socket_base::do_not_route, bool>
                                  && has_unary_settable_socket_option<T, asio::socket_base::enable_connection_aborted, bool>
                                  && has_unary_settable_socket_option<T, asio::socket_base::keep_alive, bool>
                                  && has_binary_settable_socket_option<T, asio::socket_base::linger, bool, int>
                                  && has_unary_settable_socket_option<T, asio::socket_base::out_of_band_inline, bool>
                                  && has_unary_settable_socket_option<T, asio::socket_base::receive_buffer_size, int>
                                  && has_unary_settable_socket_option<T, asio::socket_base::receive_low_watermark, int>
                                  && has_unary_settable_socket_option<T, asio::socket_base::reuse_address, bool>
                                  && has_unary_settable_socket_option<T, asio::socket_base::send_buffer_size, int>
                                  && has_unary_settable_socket_option<T, asio::socket_base::send_low_watermark, int>;
    } //namespace socket_options

    /**
     * @concept has_io_control_command
     * @brief Concept for I/O control commands.
     * @tparam T The socket type.
     * @tparam Command The command type.
     */
    template<typename T, typename Command>
    concept has_io_control_command = requires(T& temp, Command& cmd, std::error_code& ec_out) {
                                      { temp.io_control(cmd) } -> std::same_as<void>;
                                      { temp.io_control(cmd, ec_out) };
                                  };

    /**
     * @concept io_controller
     * @brief Concept for types supporting I/O control (e.g., bytes readable).
     * @tparam T The type to check.
     * @see `asio::socket_base::bytes_readable`
     */
    template<typename T>
    concept io_controller = has_io_control_command<T, asio::socket_base::bytes_readable>;

    /**
     * @concept bitmask_type
     * @brief Concept for types modeling the "BitmaskType" Named Requirement.
     * @tparam T The type to check.
     */
    template<typename T>
    concept bitmask_type = requires(T temp1, T temp2) {
                              { temp1 & temp2 } noexcept -> std::same_as<T>;
                              { temp1 | temp2 } noexcept -> std::same_as<T>;
                              { ~temp1 } noexcept -> std::same_as<T>;
                              { temp1 = temp2 } noexcept -> std::same_as<T&>;
                              { temp1 &= temp2 } noexcept -> std::same_as<T&>;
                              { temp1 |= temp2 } noexcept -> std::same_as<T&>;
                              { temp1 ^= temp2 } noexcept -> std::same_as<T&>;
                              { temp1 == temp2 } noexcept -> std::convertible_to<bool>;
                          };

    /**
     * @concept message_flag_provider
     * @brief Concept for types providing message flags for use with low-level socket operations.
     * @tparam T The type to check.
     */
    template<typename T>
    concept message_flag_provider = requires {
                                      typename T::message_flags;
                                      requires bitmask_type<typename T::message_flags>;
                                      { T::message_do_not_route } -> std::convertible_to<typename T::message_flags>;
                                      { T::message_end_of_record } -> std::convertible_to<typename T::message_flags>;
                                      { T::message_out_of_band } -> std::convertible_to<typename T::message_flags>;
                                      { T::message_peek } -> std::convertible_to<typename T::message_flags>;
                                  };

    /**
     * @concept asio_executor_associated
     * @brief Concept for types providing a valid executor via a member function.
     * @tparam T The type to check.
     * @remark This models the minimal executor-related requirement for an Asio I/O object.
     */
    template<typename T>
    concept asio_executor_associated = requires(T& temp) {
                                         { temp.get_executor() } noexcept -> asio::execution::executor;
                                     };

    /**
     * @concept asio_executor_provider
     * @brief Concept for types providing an executor compatible with Boost.Asio.
     * @tparam T The type to check.
     * @remark Requires nested `executor_type` and `get_executor()` returning compatible type.
     * @see `boost::asio::ip::tcp::socket`
     */
    template<typename T>
    concept asio_executor_provider = asio_executor_associated<T>
                                && requires(T& temp) {
                                       requires asio::execution::executor<typename T::executor_type>;
                                       { temp.get_executor() } noexcept -> std::convertible_to<typename T::executor_type>;
                                   };

    inline namespace streams {
        /**
         * @concept asio_async_read_stream
         * @brief Concept for types supporting asynchronous read operations per Boost.Asio's "AsyncReadStream" protocol.
         * @tparam T The type to check.
         * @remark Requires `async_read_some` with various completion tokens.
         * @see `boost::asio::ip::tcp::socket`
         */
        template<typename T>
        concept asio_async_read_stream = asio_executor_associated<T>
                                   && requires(T& temp, asio::mutable_buffer& buffer, asio_sample_completion_token& token) {
                                          temp.async_read_some(buffer, std::forward<asio_sample_completion_token>(token));
                                          temp.async_read_some(buffer, asio::deferred);
                                          temp.async_read_some(buffer, asio::detached);
                                          temp.async_read_some(buffer, asio::use_awaitable);
                                          temp.async_read_some(buffer, asio::use_future);
                                      };

        /**
         * @concept asio_sync_read_stream
         * @brief Concept for types supporting synchronous read operations per Boost.Asio's "SyncReadStream" protocol.
         * @tparam T The type to check.
         * @remark Requires `read_some` with and without error code.
         * @see `boost::asio::ip::tcp::socket`
         */
        template<typename T>
        concept asio_sync_read_stream = requires(T& temp, asio::mutable_buffer& buffer, std::error_code& ec_out) {
                                         { temp.read_some(buffer) } -> std::convertible_to<std::size_t>;
                                         { temp.read_some(buffer, ec_out) } -> std::convertible_to<std::size_t>;
                                     };

        /**
         * @concept asio_async_write_stream
         * @brief Concept for types supporting asynchronous write operations per Boost.Asio's "AsyncWriteStream" protocol.
         * @tparam T The type to check.
         * @remark Requires `async_write_some` with various completion tokens.
         * @see `boost::asio::ip::tcp::socket`
         */
        template<typename T>
        concept asio_async_write_stream = asio_executor_associated<T>
                                    && requires(T& temp, asio::const_buffer& buffer, asio_sample_completion_token&& token) {
                                           temp.async_write_some(buffer, std::forward<asio_sample_completion_token>(token));
                                           temp.async_write_some(buffer, asio::deferred);
                                           temp.async_write_some(buffer, asio::detached);
                                           temp.async_write_some(buffer, asio::use_awaitable);
                                           temp.async_write_some(buffer, asio::use_future);
                                       };

        /**
         * @concept asio_sync_write_stream
         * @brief Concept for types supporting synchronous write operations per Boost.Asio's "SyncWriteStream" protocol.
         * @tparam T The type to check.
         * @remark Requires `write_some` with and without error code.
         * @see `boost::asio::ip::tcp::socket`
         */
        template<typename T>
        concept asio_sync_write_stream = requires(T& temp, asio::const_buffer& buffer, std::error_code& ec_out) {
                                          { temp.write_some(buffer) } -> std::convertible_to<std::size_t>;
                                          { temp.write_some(buffer, ec_out) } -> std::convertible_to<std::size_t>;
                                      };
    } //namespace streams

    inline namespace waitables {
        /**
         * @concept asio_async_timed_waitable
         * @brief Concept for types supporting timer-style asynchronous wait operations.
         * @tparam T The type to check.
         * @see `asio::steady_timer`
         */
        template<typename T>
        concept asio_async_timed_waitable = asio_executor_associated<T>
                                      && requires(T& temp, asio_sample_completion_token&& token) {
                                             temp.async_wait(std::forward<asio_sample_completion_token>(token));
                                             temp.async_wait(asio::deferred);
                                             temp.async_wait(asio::detached);
                                             temp.async_wait(asio::use_awaitable);
                                             temp.async_wait(asio::use_future);
                                         };

        /**
         * @concept asio_sync_timed_waitable
         * @brief Concept for types supporting timer-style synchronous wait operations.
         * @tparam T The type to check.
         */
        template<typename T>
        concept asio_sync_timed_waitable = requires(T& temp, std::error_code& ec_out) {
                                            { temp.wait() } -> std::same_as<void>;
                                            { temp.wait(ec_out) };
                                        };

        /**
         * @concept has_activity_async_wait
         * @brief Helper concept for async wait with activity type.
         * @tparam T The waitable type.
         * @tparam WaitType The activity type (e.g., `wait_read`).
         */
        template<typename T, typename WaitType>
        concept has_activity_async_wait = requires(T& temp, WaitType wait, asio_sample_completion_token&& token) {
                                           temp.async_wait(wait, std::forward<asio_sample_completion_token>(token));
                                           temp.async_wait(wait, asio::deferred);
                                           temp.async_wait(wait, asio::detached);
                                           temp.async_wait(wait, asio::use_awaitable);
                                           temp.async_wait(wait, asio::use_future);
                                       };

        /**
         * @concept asio_async_activity_waitable
         * @brief Concept for types supporting activity-style asynchronous wait operations.
         * @tparam T The type to check.
         * @remark `T::wait_type` enumerates the activities (e.g., `wait_read`, `wait_write`).
         */
        template<typename T>
        concept asio_async_activity_waitable = asio_executor_associated<T>
                                         && requires {
                                                typename T::wait_type;
                                                requires has_activity_async_wait<T, typename T::wait_type>;
                                            };

        /**
         * @concept has_activity_sync_wait
         * @brief Helper concept for sync wait with activity type.
         * @tparam T The waitable type.
         * @tparam WaitType The activity type.
         */
        template<typename T, typename WaitType>
        concept has_activity_sync_wait = requires(T& temp, WaitType wait, std::error_code& ec_out) {
                                          { temp.wait(wait) } -> std::same_as<void>;
                                          { temp.wait(wait, ec_out) };
                                      };

        /**
         * @concept asio_sync_activity_waitable
         * @brief Concept for types supporting activity-style synchronous wait operations.
         * @tparam T The type to check.
         * @remark `T::wait_type` enumerates the activities.
         */
        template<typename T>
        concept asio_sync_activity_waitable = requires {
                                               typename T::wait_type;
                                               requires has_activity_sync_wait<T, typename T::wait_type>;
                                           };
    } //namespace waitables

    inline namespace socket_transmission {
        /**
         * @concept has_async_send
         * @brief Helper concept for async send with flags.
         * @tparam T The socket type.
         * @tparam MessageFlags The flag type.
         */
        template<typename T, typename MessageFlags>
        concept has_async_send =
            requires(T& temp, MessageFlags flags, asio::const_buffer& buffer, asio_sample_completion_token&& token) {
                temp.async_send(buffer, std::forward<asio_sample_completion_token>(token));
                temp.async_send(buffer, asio::deferred);
                temp.async_send(buffer, asio::detached);
                temp.async_send(buffer, asio::use_awaitable);
                temp.async_send(buffer, asio::use_future);

                temp.async_send(buffer, flags, std::forward<asio_sample_completion_token>(token));
                temp.async_send(buffer, flags, asio::deferred);
                temp.async_send(buffer, flags, asio::detached);
                temp.async_send(buffer, flags, asio::use_awaitable);
                temp.async_send(buffer, flags, asio::use_future);
            };

        /**
         * @concept asio_async_sender
         * @brief Concept for types supporting async send with message flags.
         * @tparam T The type to check.
         */
        template<typename T>
        concept asio_async_sender = asio_executor_associated<T> && requires {
                                                                   typename T::message_flags;
                                                                   requires has_async_send<T, typename T::message_flags>;
                                                               };

        /**
         * @concept has_sync_send
         * @brief Helper concept for sync send.
         * @tparam T The socket type.
         * @tparam MessageFlags The flag type.
         */
        template<typename T, typename MessageFlags>
        concept has_sync_send = requires(T& temp, asio::const_buffer buffer, MessageFlags flags, std::error_code& ec_out) {
                                  { temp.send(buffer) } -> std::convertible_to<std::size_t>;
                                  { temp.send(buffer, flags) } -> std::convertible_to<std::size_t>;
                                  { temp.send(buffer, flags, ec_out) } -> std::convertible_to<std::size_t>;
                              };

        /**
         * @concept asio_sync_sender
         * @brief Concept for types supporting sync send with message flags.
         * @tparam T The type to check.
         */
        template<typename T>
        concept asio_sync_sender = requires {
                                     typename T::message_flags;
                                     requires has_sync_send<T, typename T::message_flags>;
                                 };

        /**
         * @concept has_at_mark
         * @brief Concept for `at_mark()` support.
         * @tparam T The type to check.
         */
        template<typename T>
        concept has_at_mark = requires(T& temp, std::error_code& ec_out) {
                                { std::as_const(temp).at_mark() } -> std::convertible_to<bool>;
                                { std::as_const(temp).at_mark(ec_out) } -> std::convertible_to<bool>;
                            };

        /**
         * @concept has_available
         * @brief Concept for `available()` support.
         * @tparam T The type to check.
         */
        template<typename T>
        concept has_available = requires(T& temp, std::error_code& ec_out) {
                                   { std::as_const(temp).available() } -> std::convertible_to<std::size_t>;
                                   { std::as_const(temp).available(ec_out) } -> std::convertible_to<std::size_t>;
                               };

        /**
         * @concept has_async_receive
         * @brief Helper concept for async receive.
         * @tparam T The socket type.
         * @tparam MessageFlags The flag type.
         */
        template<typename T, typename MessageFlags>
        concept has_async_receive =
            requires(T& temp, MessageFlags flags, asio::mutable_buffer& buffer, asio_sample_completion_token&& token) {
                temp.async_receive(buffer, std::forward<asio_sample_completion_token>(token));
                temp.async_receive(buffer, asio::deferred);
                temp.async_receive(buffer, asio::detached);
                temp.async_receive(buffer, asio::use_awaitable);
                temp.async_receive(buffer, asio::use_future);

                temp.async_receive(buffer, flags, std::forward<asio_sample_completion_token>(token));
                temp.async_receive(buffer, flags, asio::deferred);
                temp.async_receive(buffer, flags, asio::detached);
                temp.async_receive(buffer, flags, asio::use_awaitable);
                temp.async_receive(buffer, flags, asio::use_future);
            };

        /**
         * @concept asio_async_receiver
         * @brief Concept for types supporting async receive with OOB and at-mark checks.
         * @tparam T The type to check.
         */
        template<typename T>
        concept asio_async_receiver = asio_executor_associated<T> && requires(T& temp, std::error_code& ec_out) {
                                                                     typename T::message_flags;
                                                                     requires has_async_receive<T, typename T::message_flags>;
                                                                     requires has_at_mark<T>;
                                                                     requires has_available<T>;
                                                                 };

        /**
         * @concept has_sync_receive
         * @brief Helper concept for sync receive.
         * @tparam T The socket type.
         * @tparam MessageFlags The flag type.
         */
        template<typename T, typename MessageFlags>
        concept has_sync_receive = requires(T& temp, asio::mutable_buffer buffer, MessageFlags flags, std::error_code& ec_out) {
                                     { temp.receive(buffer) } -> std::convertible_to<std::size_t>;
                                     { temp.receive(buffer, flags) } -> std::convertible_to<std::size_t>;
                                     { temp.receive(buffer, flags, ec_out) } -> std::convertible_to<std::size_t>;
                                 };

        /**
         * @concept asio_sync_receiver
         * @brief Concept for types supporting sync receive with OOB and at-mark checks.
         * @tparam T The type to check.
         */
        template<typename T>
        concept asio_sync_receiver = requires {
                                       typename T::message_flags;
                                       requires has_sync_receive<T, typename T::message_flags>;
                                       requires has_at_mark<T>;
                                       requires has_available<T>;
                                   };
    } //namespace socket_transmission

    /**
     * @concept counted_cancellable_resource
     * @brief Concept for types supporting cancellation with operation count.
     * @tparam T The type to check.
     */
    template<typename T>
    concept counted_cancellable_resource = requires(T& temp, std::error_code& ec_out) {
                                             { temp.cancel() } -> std::convertible_to<std::size_t>;
                                             { temp.cancel(ec_out) } -> std::convertible_to<std::size_t>;
                                         };

    /**
     * @concept uncounted_cancellable_resource
     * @brief Concept for types supporting cancellation without count.
     * @tparam T The type to check.
     */
    template<typename T>
    concept uncounted_cancellable_resource = requires(T& temp, std::error_code& ec_out) {
                                               { temp.cancel() } -> std::same_as<void>;
                                               { temp.cancel(ec_out) };
                                           };

    /**
     * @concept cancellable_resource
     * @brief Concept for types supporting cancellation of outstanding operations.
     * @tparam T The type to check.
     */
    template<typename T>
    concept cancellable_resource = counted_cancellable_resource<T> || uncounted_cancellable_resource<T>;

    /**
     * @concept closable_resource
     * @brief Concept for types that can be closed and queried for open state.
     * @tparam T The type to check.
     */
    template<typename T>
    concept closable_resource = requires(T& temp, std::error_code& ec_out) {
                                   { std::as_const(temp).is_open() } -> std::convertible_to<bool>;
                                   { temp.close() } -> std::same_as<void>;
                                   { temp.close(ec_out) };
                               };

    /**
     * @concept endpoint_provider
     * @brief Concept for types providing endpoint access.
     * @tparam T The type to check.
     * @remark Requires `local_endpoint` and `remote_endpoint` with error handling.
     * @see `boost::asio::ip::tcp::socket`
     */
    template<typename T>
    concept endpoint_provider = requires(T& temp, std::error_code& ec_out) {
                                   typename T::endpoint_type;
                                   { temp.local_endpoint() } -> std::convertible_to<typename T::endpoint_type>;
                                   { temp.local_endpoint(ec_out) } -> std::convertible_to<typename T::endpoint_type>;
                                   { temp.remote_endpoint() } -> std::convertible_to<typename T::endpoint_type>;
                                   { temp.remote_endpoint(ec_out) } -> std::convertible_to<typename T::endpoint_type>;
                               };

    inline namespace socket_connection {
        /**
         * @concept has_shutdown
         * @brief Helper concept for socket shutdown.
         * @tparam T The socket type.
         * @tparam ShutdownType The shutdown type (e.g., `shutdown_both`).
         */
        template<typename T, typename ShutdownType>
        concept has_shutdown = requires(T& temp, ShutdownType what, std::error_code& ec_out) {
                                  { temp.shutdown(what) } -> std::same_as<void>;
                                  { temp.shutdown(what, ec_out) };
                              };

        /**
         * @concept has_bind
         * @brief Helper concept for binding to local endpoint.
         * @tparam T The socket type.
         * @tparam EndpointType The endpoint type.
         */
        template<typename T, typename EndpointType>
        concept has_bind = requires(T& temp, const EndpointType& endpoint, std::error_code& ec_out) {
                              { temp.bind(endpoint) } -> std::same_as<void>;
                              { temp.bind(endpoint, ec_out) };
                          };

        /**
         * @concept has_async_connect
         * @brief Helper concept for async connect.
         * @tparam T The socket type.
         * @tparam EndpointType The endpoint type.
         */
        template<typename T, typename EndpointType>
        concept has_async_connect = requires(T& temp, EndpointType peer, asio_sample_completion_token&& token) {
                                      temp.async_connect(peer, std::forward<asio_sample_completion_token>(token));
                                      temp.async_connect(peer, asio::deferred);
                                      temp.async_connect(peer, asio::detached);
                                      temp.async_connect(peer, asio::use_awaitable);
                                      temp.async_connect(peer, asio::use_future);
                                  };

        /**
         * @concept asio_async_connectable
         * @brief Concept for types supporting async connect, bind, and shutdown.
         * @tparam T The type to check.
         */
        template<typename T>
        concept asio_async_connectable = requires {
                                           typename T::endpoint_type;
                                           typename T::shutdown_type;
                                           requires has_async_connect<T, typename T::endpoint_type>;
                                           requires has_bind<T, typename T::endpoint_type>;
                                           requires has_shutdown<T, typename T::shutdown_type>;
                                       };

        /**
         * @concept has_sync_connect
         * @brief Helper concept for sync connect.
         * @tparam T The socket type.
         * @tparam EndpointType The endpoint type.
         */
        template<typename T, typename EndpointType>
        concept has_sync_connect = requires(T& temp, const EndpointType& peer, std::error_code& ec_out) {
                                     { temp.connect(peer) } -> std::same_as<void>;
                                     { temp.connect(peer, ec_out) };
                                 };

        /**
         * @concept asio_sync_connectable
         * @brief Concept for types supporting sync connect, bind, and shutdown.
         * @tparam T The type to check.
         */
        template<typename T>
        concept asio_sync_connectable = requires {
                                          typename T::endpoint_type;
                                          typename T::shutdown_type;
                                          requires has_sync_connect<T, typename T::endpoint_type>;
                                          requires has_bind<T, typename T::endpoint_type>;
                                          requires has_shutdown<T, typename T::shutdown_type>;
                                      };
    } //namespace socket_connection

    /**
     * @concept has_native_socket_assign
     * @brief Concept for native handle assignment.
     * @tparam T The socket type.
     * @tparam Protocol The protocol type.
     * @tparam NativeHandle The native handle type.
     */
    template<typename T, typename Protocol, typename NativeHandle>
    concept has_native_socket_assign =
        requires(T& temp, const Protocol& pro, const NativeHandle& nat_hand, std::error_code& ec_out) {
            { temp.assign(pro, nat_hand) } -> std::same_as<void>;
            { temp.assign(pro, nat_hand, ec_out) };
        };

    /**
     * @concept native_socket_wrapper
     * @brief Concept for types wrapping native socket handles.
     * @tparam T The type to check.
     */
    template<typename T>
    concept native_socket_wrapper =
        requires(T& temp, std::error_code& ec_out, bool b_temp) {
            typename T::protocol_type;
            typename T::native_handle_type;
            requires has_native_socket_assign<T, typename T::protocol_type, typename T::native_handle_type>;
            { temp.native_handle() } -> std::convertible_to<typename T::native_handle_type>;
            { std::as_const(temp).native_non_blocking() } -> std::convertible_to<bool>;
            { temp.native_non_blocking(b_temp) } -> std::same_as<void>;
            { temp.native_non_blocking(b_temp, ec_out) };
            { temp.release() } -> std::convertible_to<typename T::native_handle_type>;
            { temp.release(ec_out) } -> std::convertible_to<typename T::native_handle_type>;
        };

    /**
     * @concept layerable_object
     * @brief Concept for types supporting layerable stream operations (lowest layer access).
     * @tparam T The type to check.
     * @remark Requires `lowest_layer_type` and `lowest_layer()` returning a reference.
     * @see `boost::asio::ip::tcp::socket`, `boost::asio::ssl::stream`
     */
    template<typename T>
    concept layerable_object = requires(T& temp) {
                                  typename T::lowest_layer_type;
                                  { temp.lowest_layer() } -> std::convertible_to<typename T::lowest_layer_type&>;
                                  {
                                      std::as_const(temp).lowest_layer()
                                  } -> std::convertible_to<const typename T::lowest_layer_type&>;
                              };

    /**
     * @concept layered_object
     * @brief Concept for types supporting layered stream operations (next layer access).
     * @tparam T The type to check.
     * @remark Requires `layerable_object` plus `next_layer_type` and `next_layer()`.
     * @see `boost::asio::ssl::stream`, `:stream` for `telnet::stream`
     */
    template<typename T>
    concept layered_object = layerable_object<T> && requires(T& temp) {
                                                      typename T::next_layer_type;
                                                      {
                                                          temp.next_layer()
                                                      } -> std::convertible_to<typename T::next_layer_type&>;
                                                      {
                                                          std::as_const(temp).next_layer()
                                                      } -> std::convertible_to<const typename T::next_layer_type&>;
                                                  };

    /**
     * @concept layerable_endpoint_provider
     * @brief Concept for types supporting layerable endpoint access.
     * @tparam T The type to check.
     */
    template<typename T>
    concept layerable_endpoint_provider = layerable_object<T> && endpoint_provider<typename T::lowest_layer_type>;

    /**
     * @concept asio_socket
     * @brief Concept for a complete Boost.Asio socket with all required operations.
     * @tparam T The type to check.
     */
    template<typename T>
    concept asio_socket = layerable_object<T> && asio_executor_provider<T> && native_socket_wrapper<T>
                      && uncounted_cancellable_resource<T> && closable_resource<T> && asio_async_activity_waitable<T>
                      && asio_sync_activity_waitable<T> && socket_option_provider<T> && socket_option_getter<T>
                      && socket_option_setter<T> && message_flag_provider<T> && endpoint_provider<T> && asio_async_connectable<T>
                      && asio_sync_connectable<T> && io_controller<T> && asio_async_sender<T> && asio_sync_sender<T>
                      && asio_async_receiver<T> && asio_sync_receiver<T>;

    /**
     * @concept asio_stream
     * @brief Concept for stream operations (read/write).
     * @tparam T The type to check.
     */
    template<typename T>
    concept asio_stream = asio_async_read_stream<T> && asio_sync_read_stream<T> && asio_async_write_stream<T> && asio_sync_write_stream<T>;

    /**
     * @concept asio_stream_socket
     * @brief Concept for a full stream-capable socket.
     * @tparam T The type to check.
     */
    template<typename T>
    concept asio_stream_socket = asio_socket<T> && asio_stream<T>;

    /**
     * @concept asio_layerable_socket
     * @brief Concept for layerable socket (lowest layer is full socket).
     * @tparam T The type to check.
     */
    template<typename T>
    concept asio_layerable_socket = layerable_object<T> && asio_socket<typename T::lowest_layer_type>;

    /**
     * @concept asio_layerable_stream_socket
     * @brief Concept for layerable stream socket.
     * @tparam T The type to check.
     */
    template<typename T>
    concept asio_layerable_stream_socket = layerable_object<T> && asio_stream_socket<typename T::lowest_layer_type> && asio_stream<T>;
} // namespace net::asio_concepts
