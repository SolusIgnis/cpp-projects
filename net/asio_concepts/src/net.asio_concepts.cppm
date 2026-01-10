/**
 * @file net.asio_concepts.cppm
 * @version 0.1.0
 * @release_date October 29, 2025
 *
 * @brief Module defining reusable C++20 `concept`s for Asio I/O object requirements.
 * @remark Provides fine-grained, composable concepts for Asio stream, socket, and layered types.
 * @see `asio::ip::tcp::socket`, `asio::ssl::stream`, `telnet:concepts`
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 */

module; // Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio.hpp>

// Primary module interface unit
export module net.asio_concepts;

import std; // For std::error_code, std::size_t, std::same_as, std::convertible_to

//namespace asio = boost::asio; //only if asio not standalone

namespace net::asio_concepts {
    /**
     * @typedef asio_sample_completion_token
     * Canonical default completion token with general completion executor.
     * @warning This is used to test for a CompletionToken template parameter in some `concept`s.
     */
    using asio_sample_completion_token = asio::default_completion_token_t<asio::any_completion_executor>;
    
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
} // namespace net::asio_concepts

export namespace net::asio_concepts {

    inline namespace buffers {
        /**
         * @concept AsioMutableBufferSequence
         * @brief Concept for types modeling "MutableBufferSequence".
         * @tparam T The type to check.
         * @see `boost::asio::mutable_buffer`, `std::array<mutable_buffer, N>`
         */
        template<typename T>
        concept AsioMutableBufferSequence = asio::is_mutable_buffer_sequence<T>::value;

        /**
         * @concept AsioConstBufferSequence
         * @brief Concept for types modeling "ConstBufferSequence".
         * @tparam T The type to check.
         * @see `boost::asio::const_buffer`, `std::span<const_buffer>`
         */
        template<typename T>
        concept AsioConstBufferSequence = asio::is_const_buffer_sequence<T>::value;
    } // namespace asio_concepts::buffers

    inline namespace tokens {
        /**
         * @concept AsioReadToken
         * @brief Concept for types modeling a "CompletionToken" for read operations.
         * @tparam T The type to check.
         * @see `asio::use_awaitable`, `asio::detached`
         */
        template<typename T>
        concept AsioReadToken = asio::completion_token_for<T, asio_read_completion_signature>;

        /**
         * @concept AsioWriteToken
         * @brief Concept for types modeling a "CompletionToken" for write operations.
         * @tparam T The type to check.
         * @see `asio::use_awaitable`, `asio::detached`
         */
        template<typename T>
        concept AsioWriteToken = asio::completion_token_for<T, asio_write_completion_signature>;

        /**
         * @concept AsioWaitToken
         * @brief Concept for types modeling a "CompletionToken" for wait operations.
         * @tparam T The type to check.
         * @see `asio::use_awaitable`, `asio::detached`
         */
        template<typename T>
        concept AsioWaitToken = asio::completion_token_for<T, asio_wait_completion_signature>;

        /**
         * @concept AsioConnectToken
         * @brief Concept for types modeling a "CompletionToken" for connect operations.
         * @tparam T The type to check.
         * @see `asio::use_awaitable`, `asio::detached`
         */
        template<typename T>
        concept AsioConnectToken = asio::completion_token_for<T, asio_connect_completion_signature>;
    } // namespace asio_concepts::tokens

    inline namespace socket_options {
        /**
         * @concept BooleanSocketOption
         * @brief Concept for types modeling boolean-valued socket options.
         * @tparam T The type to check.
         * @see `asio::socket_base::broadcast`, `reuse_address`
         */
        template<typename T>
        concept BooleanSocketOption =
            std::default_initializable<T> &&
            requires(T t, bool v) {
                { std::as_const(t).value() } -> std::convertible_to<bool>;
                requires std::constructible_from<T, bool>;
            };

        /**
         * @concept IntegralSocketOption
         * @brief Concept for types modeling integer-valued socket options.
         * @tparam T The type to check.
         * @see `asio::socket_base::receive_buffer_size`
         */
        template<typename T>
        concept IntegralSocketOption =
            std::default_initializable<T> &&
            requires(T t) {
                { std::as_const(t).value() };
                requires (!(std::same_as<decltype(std::as_const(t).value()), bool>));
                requires std::integral<decltype(std::as_const(t).value())>;
                requires std::constructible_from<T, decltype(std::as_const(t).value())>;
            };

        /**
         * @concept CompositeSocketOption
         * @brief Concept for types modeling composite-valued (boolean/integer pair) socket options.
         * @tparam T The type to check.
         * @see `boost::asio::socket_base::linger`
         */
        template<typename T>
        concept CompositeSocketOption =
            std::default_initializable<T> &&
            requires(T t) {
                { std::as_const(t).enabled() } -> std::convertible_to<bool>;
                { std::as_const(t).timeout() };
                requires std::integral<decltype(std::as_const(t).timeout())>;
                requires std::constructible_from<T, bool, decltype(std::as_const(t).timeout())>;
            };

        /**
         * @concept AsioAddressibleSocketOption
         * @brief Concept for types modeling address-constructed socket options.
         * @tparam T The type to check.
         * @see `asio::ip::multicast::join_group`
         */
        template<typename T>
        concept AsioAddressibleSocketOption =
            std::default_initializable<T> &&
            std::constructible_from<T, asio::ip::address>;

        /**
         * @concept SocketOption
         * @brief Concept for types modeling general socket options.
         * @tparam T The type to check.
         * @remark Allows any of the four basic option types.
         */
        template<typename T>
        concept SocketOption =
            BooleanSocketOption<T> ||
            IntegralSocketOption<T> ||
            CompositeSocketOption<T> ||
            AsioAddressibleSocketOption<T> ||
            false;

        /**
         * @concept SocketOptionProvider
         * @brief Concept for types providing the standard set of socket options.
         * @tparam T The type to check.
         * @remark Requires all standard `socket_base` option types to satisfy their respective concepts.
         */
        template<typename T>
        concept SocketOptionProvider =
            requires {
                requires BooleanSocketOption<typename T::broadcast>;
                requires BooleanSocketOption<typename T::debug>;
                requires BooleanSocketOption<typename T::do_not_route>;
                requires BooleanSocketOption<typename T::enable_connection_aborted>;
                requires BooleanSocketOption<typename T::keep_alive>;
                requires CompositeSocketOption<typename T::linger>;
                requires BooleanSocketOption<typename T::out_of_band_inline>;
                requires IntegralSocketOption<typename T::receive_buffer_size>;
                requires IntegralSocketOption<typename T::receive_low_watermark>;
                requires BooleanSocketOption<typename T::reuse_address>;
                requires IntegralSocketOption<typename T::send_buffer_size>;
                requires IntegralSocketOption<typename T::send_low_watermark>;
            };

        /**
         * @concept HasGettableSocketOption
         * @brief Concept for getting a specific socket option.
         * @tparam T The socket type.
         * @tparam Option The option type.
         * @remark Requires both error-code and non-throwing overloads.
         */
        template<typename T, typename Option>
        concept HasGettableSocketOption =
            requires(T& t, Option& opt, std::error_code& ec_out) {
                { t.get_option(opt)         }          -> std::same_as<void>;
                { t.get_option(opt, ec_out) } noexcept -> std::same_as<void>;
            };

        /**
         * @concept SocketOptionGetter
         * @brief Concept for types that support getting all standard socket options.
         * @tparam T The type to check.
         */
        template<typename T>
        concept SocketOptionGetter =
            HasGettableSocketOption<T, asio::socket_base::broadcast> &&
            HasGettableSocketOption<T, asio::socket_base::debug> &&
            HasGettableSocketOption<T, asio::socket_base::do_not_route> &&
            HasGettableSocketOption<T, asio::socket_base::enable_connection_aborted> &&
            HasGettableSocketOption<T, asio::socket_base::keep_alive> &&
            HasGettableSocketOption<T, asio::socket_base::linger> &&
            HasGettableSocketOption<T, asio::socket_base::out_of_band_inline> &&
            HasGettableSocketOption<T, asio::socket_base::receive_buffer_size> &&
            HasGettableSocketOption<T, asio::socket_base::receive_low_watermark> &&
            HasGettableSocketOption<T, asio::socket_base::reuse_address> &&
            HasGettableSocketOption<T, asio::socket_base::send_buffer_size> &&
            HasGettableSocketOption<T, asio::socket_base::send_low_watermark> &&
            true;

        /**
         * @concept HasUnarySettableSocketOption
         * @brief Concept for setting a unary socket option.
         * @tparam T The socket type.
         * @tparam Option The option type.
         * @tparam Arg The argument type.
         */
        template<typename T, typename Option, typename Arg>
        concept HasUnarySettableSocketOption =
            requires(T& t, std::error_code& ec_out, Arg& a) {
                { t.set_option(Option(a))         } -> std::same_as<void>;
                { t.set_option(Option(a), ec_out) } -> std::same_as<void>;
            };

        /**
         * @concept HasBinarySettableSocketOption
         * @brief Concept for setting a binary socket option.
         * @tparam T The socket type.
         * @tparam Option The option type.
         * @tparam Arg1, Arg2 Argument types.
         */
        template<typename T, typename Option, typename Arg1, typename Arg2>
        concept HasBinarySettableSocketOption =
            requires(T& t, std::error_code& ec_out, Arg1 a1, Arg2 a2) {
                { t.set_option(Option(a1, a2))         } -> std::same_as<void>;
                { t.set_option(Option(a1, a2), ec_out) } -> std::same_as<void>;
            };

        /**
         * @concept SocketOptionSetter
         * @brief Concept for types that support setting all standard socket options.
         * @tparam T The type to check.
         */
        template<typename T>
        concept SocketOptionSetter =
            HasUnarySettableSocketOption<T, asio::socket_base::broadcast, bool> &&
            HasUnarySettableSocketOption<T, asio::socket_base::debug, bool> &&
            HasUnarySettableSocketOption<T, asio::socket_base::do_not_route, bool> &&
            HasUnarySettableSocketOption<T, asio::socket_base::enable_connection_aborted, bool> &&
            HasUnarySettableSocketOption<T, asio::socket_base::keep_alive, bool> &&
            HasBinarySettableSocketOption<T, asio::socket_base::linger, bool, int> &&
            HasUnarySettableSocketOption<T, asio::socket_base::out_of_band_inline, bool> &&
            HasUnarySettableSocketOption<T, asio::socket_base::receive_buffer_size, int> &&
            HasUnarySettableSocketOption<T, asio::socket_base::receive_low_watermark, int> &&
            HasUnarySettableSocketOption<T, asio::socket_base::reuse_address, bool> &&
            HasUnarySettableSocketOption<T, asio::socket_base::send_buffer_size, int> &&
            HasUnarySettableSocketOption<T, asio::socket_base::send_low_watermark, int> &&
            true;
    } // namespace asio_concepts::socket_options

    /**
     * @concept HasIOControlCommand
     * @brief Concept for I/O control commands.
     * @tparam T The socket type.
     * @tparam Command The command type.
     */
    template<typename T, typename Command>
    concept HasIOControlCommand =
        requires(T& t, Command& cmd, std::error_code& ec_out) {
            { t.io_control(cmd)         } -> std::same_as<void>;
            { t.io_control(cmd, ec_out) } -> std::same_as<void>;
        };

    /**
     * @concept IOController
     * @brief Concept for types supporting I/O control (e.g., bytes readable).
     * @tparam T The type to check.
     * @see `asio::socket_base::bytes_readable`
     */
    template<typename T>
    concept IOController =
        HasIOControlCommand<T, asio::socket_base::bytes_readable> &&
        true;

    /**
     * @concept BitmaskType
     * @brief Concept for types modeling the "BitmaskType" Named Requirement.
     * @tparam T The type to check.
     */
    template<typename T>
    concept BitmaskType =
        requires(T t1, T t2) {
            { t1 &  t2 } noexcept -> std::same_as<T>;
            { t1 |  t2 } noexcept -> std::same_as<T>;
            { ~t1      } noexcept -> std::same_as<T>;
            { t1  = t2 } noexcept -> std::same_as<T&>;
            { t1 &= t2 } noexcept -> std::same_as<T&>;
            { t1 |= t2 } noexcept -> std::same_as<T&>;
            { t1 ^= t2 } noexcept -> std::same_as<T&>;
            { t1 == t2 } noexcept -> std::convertible_to<bool>;
        };

    /**
     * @concept MessageFlagProvider
     * @brief Concept for types providing message flags for use with low-level socket operations.
     * @tparam T The type to check.
     */
    template<typename T>
    concept MessageFlagProvider =
        requires {
            typename T::message_flags;
            requires BitmaskType<typename T::message_flags>;
            { T::message_do_not_route  } -> std::convertible_to<typename T::message_flags>;
            { T::message_end_of_record } -> std::convertible_to<typename T::message_flags>;
            { T::message_out_of_band   } -> std::convertible_to<typename T::message_flags>;
            { T::message_peek          } -> std::convertible_to<typename T::message_flags>;
        };

    /**
     * @concept AsioExecutorAssociated
     * @brief Concept for types providing a valid executor via a member function.
     * @tparam T The type to check.
     * @remark This models the minimal executor-related requirement for an Asio I/O object.
     */
    template<typename T>
    concept AsioExecutorAssociated =
        requires(T& t) {
            { t.get_executor() } noexcept -> asio::execution::executor;
        };

    /**
     * @concept AsioExecutorProvider
     * @brief Concept for types providing an executor compatible with Boost.Asio.
     * @tparam T The type to check.
     * @remark Requires nested `executor_type` and `get_executor()` returning compatible type.
     * @see `boost::asio::ip::tcp::socket`
     */
    template<typename T>
    concept AsioExecutorProvider =
        AsioExecutorAssociated<T> &&
        requires(T& t) {
            requires asio::execution::executor<typename T::executor_type>;
            { t.get_executor() } noexcept -> std::convertible_to<typename T::executor_type>;
        };

    inline namespace streams {
        /**
         * @concept AsioAsyncReadStream
         * @brief Concept for types supporting asynchronous read operations per Boost.Asio's "AsyncReadStream" protocol.
         * @tparam T The type to check.
         * @remark Requires `async_read_some` with various completion tokens.
         * @see `boost::asio::ip::tcp::socket`
         */
        template<typename T>
        concept AsioAsyncReadStream =
            AsioExecutorAssociated<T> &&
            requires(T& t, asio::mutable_buffer mbs, asio_sample_completion_token&& token) {
                t.async_read_some(mbs, std::forward<asio_sample_completion_token>(token));
                t.async_read_some(mbs, asio::deferred);
                t.async_read_some(mbs, asio::detached);
                t.async_read_some(mbs, asio::use_awaitable);
                t.async_read_some(mbs, asio::use_future);
            };

        /**
         * @concept AsioSyncReadStream
         * @brief Concept for types supporting synchronous read operations per Boost.Asio's "SyncReadStream" protocol.
         * @tparam T The type to check.
         * @remark Requires `read_some` with and without error code.
         * @see `boost::asio::ip::tcp::socket`
         */
        template<typename T>
        concept AsioSyncReadStream =
            requires(T& t, asio::mutable_buffer mbs, std::error_code& ec_out) {
                { t.read_some(mbs)         } -> std::convertible_to<std::size_t>;
                { t.read_some(mbs, ec_out) } -> std::convertible_to<std::size_t>;
            };

        /**
         * @concept AsioAsyncWriteStream
         * @brief Concept for types supporting asynchronous write operations per Boost.Asio's "AsyncWriteStream" protocol.
         * @tparam T The type to check.
         * @remark Requires `async_write_some` with various completion tokens.
         * @see `boost::asio::ip::tcp::socket`
         */
        template<typename T>
        concept AsioAsyncWriteStream =
            AsioExecutorAssociated<T> &&
            requires(T& t, asio::const_buffer cbs, asio_sample_completion_token&& token) {
                t.async_write_some(cbs, std::forward<asio_sample_completion_token>(token));
                t.async_write_some(cbs, asio::deferred);
                t.async_write_some(cbs, asio::detached);
                t.async_write_some(cbs, asio::use_awaitable);
                t.async_write_some(cbs, asio::use_future);
            };

        /**
         * @concept AsioSyncWriteStream
         * @brief Concept for types supporting synchronous write operations per Boost.Asio's "SyncWriteStream" protocol.
         * @tparam T The type to check.
         * @remark Requires `write_some` with and without error code.
         * @see `boost::asio::ip::tcp::socket`
         */
        template<typename T>
        concept AsioSyncWriteStream =
            requires(T& t, asio::const_buffer cbs, std::error_code& ec_out) {
                { t.write_some(cbs)         } -> std::convertible_to<std::size_t>;
                { t.write_some(cbs, ec_out) } -> std::convertible_to<std::size_t>;
            };
    } // namespace asio_concepts::streams

    inline namespace waitables {
        /**
         * @concept AsioAsyncTimedWaitable
         * @brief Concept for types supporting timer-style asynchronous wait operations.
         * @tparam T The type to check.
         * @see `asio::steady_timer`
         */
        template<typename T>
        concept AsioAsyncTimedWaitable =
            AsioExecutorAssociated<T> &&
            requires (T& t, asio_sample_completion_token&& token) {
                t.async_wait(std::forward<asio_sample_completion_token>(token));
                t.async_wait(asio::deferred);
                t.async_wait(asio::detached);
                t.async_wait(asio::use_awaitable);
                t.async_wait(asio::use_future);
            };

        /**
         * @concept AsioSyncTimedWaitable
         * @brief Concept for types supporting timer-style synchronous wait operations.
         * @tparam T The type to check.
         */
        template<typename T>
        concept AsioSyncTimedWaitable =
            requires (T& t, std::error_code& ec_out) {
                { t.wait()       } -> std::same_as<void>;
                { t.wait(ec_out) } -> std::same_as<void>;
            };

        /**
         * @concept HasActivityAsyncWait
         * @brief Helper concept for async wait with activity type.
         * @tparam T The waitable type.
         * @tparam WaitType The activity type (e.g., `wait_read`).
         */
        template<typename T, typename WaitType>
        concept HasActivityAsyncWait =
            requires(T& t, WaitType w, asio_sample_completion_token&& token) {
                    t.async_wait(w, std::forward<asio_sample_completion_token>(token));
                    t.async_wait(w, asio::deferred);
                    t.async_wait(w, asio::detached);
                    t.async_wait(w, asio::use_awaitable);
                    t.async_wait(w, asio::use_future);
                };

        /**
         * @concept AsioAsyncActivityWaitable
         * @brief Concept for types supporting activity-style asynchronous wait operations.
         * @tparam T The type to check.
         * @remark `T::wait_type` enumerates the activities (e.g., `wait_read`, `wait_write`).
         */
        template<typename T>
        concept AsioAsyncActivityWaitable =
            AsioExecutorAssociated<T> &&
            requires {
                typename T::wait_type;
                requires HasActivityAsyncWait<T, typename T::wait_type>;
            };

        /**
         * @concept HasActivitySyncWait
         * @brief Helper concept for sync wait with activity type.
         * @tparam T The waitable type.
         * @tparam WaitType The activity type.
         */
        template<typename T, typename WaitType>
        concept HasActivitySyncWait =
            requires(T& t, WaitType w, std::error_code& ec_out) {
                { t.wait(w)         } -> std::same_as<void>;
                { t.wait(w, ec_out) } -> std::same_as<void>;
            };

        /**
         * @concept AsioSyncActivityWaitable
         * @brief Concept for types supporting activity-style synchronous wait operations.
         * @tparam T The type to check.
         * @remark `T::wait_type` enumerates the activities.
         */
        template<typename T>
        concept AsioSyncActivityWaitable =
            requires {
                typename T::wait_type;
                requires HasActivitySyncWait<T, typename T::wait_type>;
            };
    } // namespace asio_concepts::waitables

    inline namespace socket_transmission {
        /**
         * @concept HasAsyncSend
         * @brief Helper concept for async send with flags.
         * @tparam T The socket type.
         * @tparam MessageFlags The flag type.
         */
        template<typename T, typename MessageFlags>
        concept HasAsyncSend =
            requires(T& t, asio::const_buffer cbs, MessageFlags flags, asio_sample_completion_token&& token) {
                t.async_send(cbs, std::forward<asio_sample_completion_token>(token));
                t.async_send(cbs, asio::deferred);
                t.async_send(cbs, asio::detached);
                t.async_send(cbs, asio::use_awaitable);
                t.async_send(cbs, asio::use_future);

                t.async_send(cbs, flags, std::forward<asio_sample_completion_token>(token));
                t.async_send(cbs, flags, asio::deferred);
                t.async_send(cbs, flags, asio::detached);
                t.async_send(cbs, flags, asio::use_awaitable);
                t.async_send(cbs, flags, asio::use_future);
            };

        /**
         * @concept AsioAsyncSender
         * @brief Concept for types supporting async send with message flags.
         * @tparam T The type to check.
         */
        template<typename T>
        concept AsioAsyncSender =
            AsioExecutorAssociated<T> &&
            requires {
                typename T::message_flags;
                requires HasAsyncSend<T, typename T::message_flags>;
            };

        /**
         * @concept HasSyncSend
         * @brief Helper concept for sync send.
         * @tparam T The socket type.
         * @tparam MessageFlags The flag type.
         */
        template<typename T, typename MessageFlags>
        concept HasSyncSend =
            requires(T& t, asio::const_buffer cbs, MessageFlags flags, std::error_code& ec_out) {
                { t.send(cbs)                } -> std::convertible_to<std::size_t>;
                { t.send(cbs, flags)         } -> std::convertible_to<std::size_t>;
                { t.send(cbs, flags, ec_out) } -> std::convertible_to<std::size_t>;
            };

        /**
         * @concept AsioSyncSender
         * @brief Concept for types supporting sync send with message flags.
         * @tparam T The type to check.
         */
        template<typename T>
        concept AsioSyncSender =
            requires {
                typename T::message_flags;
                requires HasSyncSend<T, typename T::message_flags>;
            };

        /**
         * @concept HasAtMark
         * @brief Concept for `at_mark()` support.
         * @tparam T The type to check.
         */
        template<typename T>
        concept HasAtMark =
            requires(T& t, std::error_code& ec_out) {
                { std::as_const(t).at_mark()       } -> std::convertible_to<bool>;
                { std::as_const(t).at_mark(ec_out) } -> std::convertible_to<bool>;
            };

        /**
         * @concept HasAvailable
         * @brief Concept for `available()` support.
         * @tparam T The type to check.
         */
        template<typename T>
        concept HasAvailable =
            requires(T& t, std::error_code& ec_out) {
                { std::as_const(t).available()       } -> std::convertible_to<std::size_t>;
                { std::as_const(t).available(ec_out) } -> std::convertible_to<std::size_t>;
            };

        /**
         * @concept HasAsyncReceive
         * @brief Helper concept for async receive.
         * @tparam T The socket type.
         * @tparam MessageFlags The flag type.
         */
        template<typename T, typename MessageFlags>
        concept HasAsyncReceive =
            requires(T& t, asio::mutable_buffer mbs, MessageFlags flags, asio_sample_completion_token&& token) {
                t.async_receive(mbs, std::forward<asio_sample_completion_token>(token));
                t.async_receive(mbs, asio::deferred);
                t.async_receive(mbs, asio::detached);
                t.async_receive(mbs, asio::use_awaitable);
                t.async_receive(mbs, asio::use_future);

                t.async_receive(mbs, flags, std::forward<asio_sample_completion_token>(token));
                t.async_receive(mbs, flags, asio::deferred);
                t.async_receive(mbs, flags, asio::detached);
                t.async_receive(mbs, flags, asio::use_awaitable);
                t.async_receive(mbs, flags, asio::use_future);
            };

        /**
         * @concept AsioAsyncReceiver
         * @brief Concept for types supporting async receive with OOB and at-mark checks.
         * @tparam T The type to check.
         */
        template<typename T>
        concept AsioAsyncReceiver =
            AsioExecutorAssociated<T> &&
            requires(T& t, std::error_code& ec_out) {
                typename T::message_flags;
                requires HasAsyncReceive<T, typename T::message_flags>;
                requires HasAtMark<T>;
                requires HasAvailable<T>;
            };

        /**
         * @concept HasSyncReceive
         * @brief Helper concept for sync receive.
         * @tparam T The socket type.
         * @tparam MessageFlags The flag type.
         */
        template<typename T, typename MessageFlags>
        concept HasSyncReceive =
            requires(T& t, asio::mutable_buffer mbs, MessageFlags flags, std::error_code& ec_out) {
                { t.receive(mbs)                   } -> std::convertible_to<std::size_t>;
                { t.receive(mbs, flags)            } -> std::convertible_to<std::size_t>;
                { t.receive(mbs, flags, ec_out)    } -> std::convertible_to<std::size_t>;
            };

        /**
         * @concept AsioSyncReceiver
         * @brief Concept for types supporting sync receive with OOB and at-mark checks.
         * @tparam T The type to check.
         */
        template<typename T>
        concept AsioSyncReceiver =
            requires {
                typename T::message_flags;
                requires HasSyncReceive<T, typename T::message_flags>;
                requires HasAtMark<T>;
                requires HasAvailable<T>;
            };
    } // namespace asio_concepts::socket_transmission

    /**
     * @concept CountedCancellableResource
     * @brief Concept for types supporting cancellation with operation count.
     * @tparam T The type to check.
     */
    template<typename T>
    concept CountedCancellableResource =
        requires(T& t, std::error_code& ec_out) {
            { t.cancel()       } -> std::convertible_to<std::size_t>;
            { t.cancel(ec_out) } -> std::convertible_to<std::size_t>;
        };

    /**
     * @concept UncountedCancellableResource
     * @brief Concept for types supporting cancellation without count.
     * @tparam T The type to check.
     */
    template<typename T>
    concept UncountedCancellableResource =
        requires(T& t, std::error_code& ec_out) {
            { t.cancel()       } -> std::same_as<void>;
            { t.cancel(ec_out) } -> std::same_as<void>;
        };

    /**
     * @concept CancellableResource
     * @brief Concept for types supporting cancellation of outstanding operations.
     * @tparam T The type to check.
     */
    template<typename T>
    concept CancellableResource =
       CountedCancellableResource<T> ||
       UncountedCancellableResource<T> ||
       false;

    /**
     * @concept ClosableResource
     * @brief Concept for types that can be closed and queried for open state.
     * @tparam T The type to check.
     */
    template<typename T>
    concept ClosableResource =
        requires(T& t, std::error_code& ec_out) {
            { std::as_const(t).is_open() } noexcept -> std::convertible_to<bool>;
            { t.close()                  }          -> std::same_as<void>;
            { t.close(ec_out)            }          -> std::same_as<void>;
        };

    /**
     * @concept EndpointProvider
     * @brief Concept for types providing endpoint access.
     * @tparam T The type to check.
     * @remark Requires `local_endpoint` and `remote_endpoint` with error handling.
     * @see `boost::asio::ip::tcp::socket`
     */
    template<typename T>
    concept EndpointProvider =
        requires(T& t, std::error_code& ec_out) {
            typename T::endpoint_type;
            { t.local_endpoint()        } -> std::convertible_to<typename T::endpoint_type>;
            { t.local_endpoint(ec_out)  } -> std::convertible_to<typename T::endpoint_type>;
            { t.remote_endpoint()       } -> std::convertible_to<typename T::endpoint_type>;
            { t.remote_endpoint(ec_out) } -> std::convertible_to<typename T::endpoint_type>;
        };

    inline namespace socket_connection {
        /**
         * @concept HasShutdown
         * @brief Helper concept for socket shutdown.
         * @tparam T The socket type.
         * @tparam ShutdownType The shutdown type (e.g., `shutdown_both`).
         */
        template<typename T, typename ShutdownType>
        concept HasShutdown =
            requires(T& t, ShutdownType what, std::error_code& ec_out) {
                { t.shutdown(what)         } -> std::same_as<void>;
                { t.shutdown(what, ec_out) } -> std::same_as<void>;
            };

        /**
         * @concept HasBind
         * @brief Helper concept for binding to local endpoint.
         * @tparam T The socket type.
         * @tparam EndpointType The endpoint type.
         */
        template<typename T, typename EndpointType>
        concept HasBind =
            requires(T& t, const EndpointType& endpoint, std::error_code& ec_out) {
                { t.bind(endpoint)         } -> std::same_as<void>;
                { t.bind(endpoint, ec_out) } -> std::same_as<void>;
            };

        /**
         * @concept HasAsyncConnect
         * @brief Helper concept for async connect.
         * @tparam T The socket type.
         * @tparam EndpointType The endpoint type.
         */
        template<typename T, typename EndpointType>
        concept HasAsyncConnect =
            requires(T& t, EndpointType peer, asio_sample_completion_token&& token) {
                    t.async_connect(peer, std::forward<asio_sample_completion_token>(token));
                    t.async_connect(peer, asio::deferred);
                    t.async_connect(peer, asio::detached);
                    t.async_connect(peer, asio::use_awaitable);
                    t.async_connect(peer, asio::use_future);
                };

        /**
         * @concept AsioAsyncConnectable
         * @brief Concept for types supporting async connect, bind, and shutdown.
         * @tparam T The type to check.
         */
        template<typename T>
        concept AsioAsyncConnectable =
            requires {
                typename T::endpoint_type;
                typename T::shutdown_type;
                requires HasAsyncConnect<T, typename T::endpoint_type>;
                requires HasBind<T, typename T::endpoint_type>;
                requires HasShutdown<T, typename T::shutdown_type>;
            };

        /**
         * @concept HasSyncConnect
         * @brief Helper concept for sync connect.
         * @tparam T The socket type.
         * @tparam EndpointType The endpoint type.
         */
        template<typename T, typename EndpointType>
        concept HasSyncConnect =
            requires(T& t, const EndpointType& peer, std::error_code& ec_out) {
                { t.connect(peer)         } -> std::same_as<void>;
                { t.connect(peer, ec_out) } -> std::same_as<void>;
            };

        /**
         * @concept AsioSyncConnectable
         * @brief Concept for types supporting sync connect, bind, and shutdown.
         * @tparam T The type to check.
         */
        template<typename T>
        concept AsioSyncConnectable =
            requires {
                typename T::endpoint_type;
                typename T::shutdown_type;
                requires HasSyncConnect<T, typename T::endpoint_type>;
                requires HasBind<T, typename T::endpoint_type>;
                requires HasShutdown<T, typename T::shutdown_type>;
            };
    } // namespace asio_concepts::socket_connection

    /**
     * @concept HasNativeSocketAssign
     * @brief Concept for native handle assignment.
     * @tparam T The socket type.
     * @tparam Protocol The protocol type.
     * @tparam NativeHandle The native handle type.
     */
    template<typename T, typename Protocol, typename NativeHandle>
    concept HasNativeSocketAssign =
        requires(T& t, const Protocol& p, const NativeHandle& n, std::error_code& ec_out) {
            { t.assign(p, n)         } -> std::same_as<void>;
            { t.assign(p, n, ec_out) } -> std::same_as<void>;
        };

    /**
     * @concept NativeSocketWrapper
     * @brief Concept for types wrapping native socket handles.
     * @tparam T The type to check.
     */
    template<typename T>
    concept NativeSocketWrapper =
        requires(T& t, std::error_code& ec_out, bool b) {
            typename T::protocol_type;
            typename T::native_handle_type;
            requires HasNativeSocketAssign<T, typename T::protocol_type, typename T::native_handle_type>;
            { t.native_handle()                      } -> std::convertible_to<typename T::native_handle_type>;
            { std::as_const(t).native_non_blocking() } -> std::convertible_to<bool>;
            { t.native_non_blocking(b)               } -> std::same_as<void>;
            { t.native_non_blocking(b, ec_out)       } -> std::same_as<void>;
            { t.release()                            } -> std::convertible_to<typename T::native_handle_type>;
            { t.release(ec_out)                      } -> std::convertible_to<typename T::native_handle_type>;
        };

    /**
     * @concept LayerableObject
     * @brief Concept for types supporting layerable stream operations (lowest layer access).
     * @tparam T The type to check.
     * @remark Requires `lowest_layer_type` and `lowest_layer()` returning a reference.
     * @see `boost::asio::ip::tcp::socket`, `boost::asio::ssl::stream`
     */
    template<typename T>
    concept LayerableObject =
        requires(T& t) {
            typename T::lowest_layer_type;
            { t.lowest_layer()                } noexcept -> std::convertible_to<typename T::lowest_layer_type&>;
            { std::as_const(t).lowest_layer() } noexcept -> std::convertible_to<const typename T::lowest_layer_type&>;
        };

    /**
     * @concept LayeredObject
     * @brief Concept for types supporting layered stream operations (next layer access).
     * @tparam T The type to check.
     * @remark Requires `LayerableObject` plus `next_layer_type` and `next_layer()`.
     * @see `boost::asio::ssl::stream`, `:stream` for `telnet::stream`
     */
    template<typename T>
    concept LayeredObject =
        LayerableObject<T> &&
        requires(T& t) {
            typename T::next_layer_type;
            { t.next_layer()                } noexcept -> std::convertible_to<typename T::next_layer_type&>;
            { std::as_const(t).next_layer() } noexcept -> std::convertible_to<const typename T::next_layer_type&>;
        };

    /**
     * @concept LayerableEndpointProvider
     * @brief Concept for types supporting layerable endpoint access.
     * @tparam T The type to check.
     */
    template<typename T>
    concept LayerableEndpointProvider =
        LayerableObject<T> &&
        EndpointProvider<typename T::lowest_layer_type> &&
        true;

    /**
     * @concept AsioSocket
     * @brief Concept for a complete Boost.Asio socket with all required operations.
     * @tparam T The type to check.
     */
    template<typename T>
    concept AsioSocket = 
        LayerableObject<T> &&
        AsioExecutorProvider<T> &&
        NativeSocketWrapper<T> &&
        UncountedCancellableResource<T> &&
        ClosableResource<T> &&
        AsioAsyncActivityWaitable<T> &&
        AsioSyncActivityWaitable<T> &&
        SocketOptionProvider<T> &&
        SocketOptionGetter<T> &&
        SocketOptionSetter<T> &&
        MessageFlagProvider<T> &&
        EndpointProvider<T> &&
        AsioAsyncConnectable<T> &&
        AsioSyncConnectable<T> &&
        IOController<T> &&
        AsioAsyncSender<T> &&
        AsioSyncSender<T> &&
        AsioAsyncReceiver<T> &&
        AsioSyncReceiver<T> &&
        true;

    /**
     * @concept AsioStream
     * @brief Concept for stream operations (read/write).
     * @tparam T The type to check.
     */
    template<typename T>
    concept AsioStream =
        AsioAsyncReadStream<T> &&
        AsioSyncReadStream<T> &&
        AsioAsyncWriteStream<T> &&
        AsioSyncWriteStream<T> &&
        true;

    /**
     * @concept AsioStreamSocket
     * @brief Concept for a full stream-capable socket.
     * @tparam T The type to check.
     */
    template<typename T>
    concept AsioStreamSocket =
        AsioSocket<T> &&
        AsioStream<T> &&
        true;

    /**
     * @concept AsioLayerableSocket
     * @brief Concept for layerable socket (lowest layer is full socket).
     * @tparam T The type to check.
     */
    template<typename T>
    concept AsioLayerableSocket =
        LayerableObject<T> &&
        AsioSocket<typename T::lowest_layer_type> &&
        true;

    /**
     * @concept AsioLayerableStreamSocket
     * @brief Concept for layerable stream socket.
     * @tparam T The type to check.
     */
    template<typename T>
    concept AsioLayerableStreamSocket =
        LayerableObject<T> &&
        AsioStreamSocket<typename T::lowest_layer_type> &&
        AsioStream<T> &&
        true;
} // namespace net::asio_concepts
