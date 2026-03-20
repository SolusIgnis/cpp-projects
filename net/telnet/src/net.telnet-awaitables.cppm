// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
/**
 * @file net.telnet-awaitables.cppm
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
 * @brief Partition defining tagged awaitable types for type-safe asynchronous operations in the Telnet library.
 * @remark Defines `tagged_awaitable` and associated semantic tag structs for option negotiation and subnegotiation handlers.
 *
 * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, `:protocol_fsm` for handler usage, `:internal` for handler definitions
 */

module; //Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio/awaitable.hpp>

// Module partition interface unit
export module net.telnet:awaitables;

import std; //NOLINT for std::move

export import :options; ///< @see "net.telnet-options.cppm" for `option`

namespace net::telnet::awaitables {
    struct adl_lookup_tag {};

    //Delete free `operator co_await` for ADL purposes.
    void operator co_await(adl_lookup_tag) = delete;
} //namespace net::telnet::awaitables

export namespace net::telnet::awaitables {
    /**
     * @brief Wrapper for an awaitable with a semantic tag for type safety.
     * @tparam Tag The semantic tag type.
     * @tparam T The awaitable's value type (i.e. the "return type" of `co_await`ing it) (e.g., `void`, `std::size_t`).
     * @tparam AwaitableT The underlying `Awaitable` type (default: `boost::asio::awaitable<T>`).
     * @remark Provides implicit conversion to/from the underlying awaitable and supports direct `co_await`.
     * @see `tags` namespace for semantic tag types, `:protocol_fsm`, `:internal`
     */
    template<typename Tag, typename T, typename AwaitableT = asio::awaitable<T>>
    class tagged_awaitable {
    private:
        using awaitable_type = AwaitableT; ///< Underlying awaitable type

        awaitable_type awaitable_; ///< The wrapped awaitable

    public:
        //NOLINTBEGIN(google-explicit-constructor): Implicit conversion to/from our `awaitable_type` is the point.

        ///@brief Default constructor.
        tagged_awaitable() = default;

        ///@brief Constructs from an awaitable.
        tagged_awaitable(awaitable_type awaitable) noexcept : awaitable_(std::move(awaitable)) {}

        ///@brief Implicit conversion to underlying awaitable (lvalue).
        operator awaitable_type() & noexcept { return awaitable_; }

        ///@brief Implicit conversion to underlying awaitable (const lvalue).
        operator awaitable_type() const& noexcept { return awaitable_; }

        ///@brief Implicit conversion to underlying awaitable (rvalue).
        operator awaitable_type() && noexcept { return std::move(awaitable_); }

        //NOLINTEND(google-explicit-constructor)

        ///@brief Explicit conversion to underlying awaitable (lvalue).
        awaitable_type& get() & noexcept { return awaitable_; }

        ///@brief Explicit conversion to underlying awaitable (const lvalue).
        const awaitable_type& get() const& noexcept { return awaitable_; }

        ///@brief Explicit conversion to underlying awaitable (rvalue).
        awaitable_type&& get() && noexcept { return std::move(awaitable_); }

        ///@brief Supports co_await for lvalue.
        decltype(auto) operator co_await() & noexcept
        {
            if constexpr (requires(awaitable_type& awaitable) { awaitable.operator co_await(); }) {
                return awaitable_.operator co_await();
            } else if constexpr (requires(awaitable_type& awaitable) { operator co_await(awaitable); }) {
                return operator co_await(awaitable_);
            } else {
                return awaitable_;
            }
        }

        ///@brief Supports co_await for const lvalue.
        decltype(auto) operator co_await() const& noexcept
        {
            if constexpr (requires(const awaitable_type& awaitable) { awaitable.operator co_await(); }) {
                return awaitable_.operator co_await();
            } else if constexpr (requires(const awaitable_type& awaitable) { operator co_await(awaitable); }) {
                return operator co_await(awaitable_);
            } else {
                return awaitable_;
            }
        }

        ///@brief Supports co_await for rvalue.
        decltype(auto) operator co_await() && noexcept
        {
            if constexpr (requires(awaitable_type&& awaitable) { std::move(awaitable).operator co_await(); }) {
                return std::move(awaitable_).operator co_await();
            } else if constexpr (requires(awaitable_type&& awaitable) { operator co_await(std::move(awaitable)); }) {
                return operator co_await(std::move(awaitable_));
            } else {
                return std::move(awaitable_);
            }
        }
    }; //class tagged_awaitable

    /**
     * @fn tagged_awaitable::tagged_awaitable(awaitable_type awaitable) noexcept
     * @param awaitable The awaitable to wrap.
     * @note Implicit conversion from the underlying type allows direct returns from Boost.Asio asynchronous operations.
     */

    /// @brief Semantic tag `struct`s to specialize `tagged_awaitable`. @see `tagged_awaitable`
    namespace tags {
        /// @brief Tag to specialize `tagged_awaitable` for option enablement handlers. @see `tagged_awaitable`
        struct option_enablement_tag {};

        /// @brief Tag to specialize `tagged_awaitable` for option disablement handlers. @see `tagged_awaitable`
        struct option_disablement_tag {};

        /// @brief Tag to specialize `tagged_awaitable` for subnegotiation handlers. @see `tagged_awaitable`
        struct subnegotiation_tag {};
    } //namespace tags

    /**
     * @typedef option_enablement_awaitable
     * @brief Awaitable type for option enablement handlers.
     * @see `tagged_awaitable`, `tags::option_enablement_tag`, `:internal` (`option_handler_registry`), `:protocol_fsm` (for use)
     */
    using option_enablement_awaitable = tagged_awaitable<tags::option_enablement_tag, void>;

    /**
     * @typedef option_disablement_awaitable
     * @brief Awaitable type for option disablement handlers.
     * @see `tagged_awaitable`, `tags::option_disablement_tag`, `:internal` (`option_handler_registry`), `:protocol_fsm` (for use)
     */
    using option_disablement_awaitable = tagged_awaitable<tags::option_disablement_tag, void>;

    /**
     * @typedef subnegotiation_awaitable
     * @brief Awaitable type for subnegotiation handlers.
     * @see `tagged_awaitable`, `tags::subnegotiation_tag`, `:internal` (`option_handler_registry`), `:protocol_fsm` (for use)
     */
    using subnegotiation_awaitable = tagged_awaitable<tags::subnegotiation_tag, std::tuple<option, std::vector<byte_t>>>;
} //namespace net::telnet::awaitables

namespace std {
    ///@brief Partial specialization of `std::coroutine_traits` forwarding the promise type for a `tagged_awaitable` to the promise type of its underlying awaitable type.
    template<typename Tag, typename T, typename AwaitableT, typename... Args>
    struct coroutine_traits<
        net::telnet::awaitables::tagged_awaitable<Tag, T, AwaitableT>,
        Args...
    > {
        using promise_type = typename std::coroutine_traits<AwaitableT, Args...>::promise_type;
    };
} //namespace std