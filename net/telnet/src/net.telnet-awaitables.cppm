// SPDX-License-Identifier: Apache-2.0
/**
 * @file net.telnet-awaitables.cppm
 * @version 0.5.7
 * @date October 30, 2025
 *
 * @copyright Copyright (c) 2025-2026 Jeremy Murphy and any Contributors
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

//namespace asio = boost::asio;

export namespace net::telnet::awaitables {
    /**
     * @brief Wrapper for an awaitable with a semantic tag for type safety.
     * @tparam Tag The semantic tag type.
     * @tparam T The awaitable's value type (i.e. the "return type" of `co_await`ing it) (e.g., `void`).
     * @tparam Awaitable The underlying awaitable type (default: `boost::asio::awaitable<T>`).
     * @remark Provides implicit conversion to/from the underlying awaitable and supports direct `co_await`.
     * @see `tags` namespace for semantic tag types, `:protocol_fsm`, `:internal`
     */
    template<typename Tag, typename T, typename Awaitable = asio::awaitable<T>>
    class tagged_awaitable {
    private:
        using awaitable_type = Awaitable; ///< Underlying awaitable type

        awaitable_type awaitable_; ///< The wrapped awaitable

    public:
        //NOLINTBEGIN(google-explicit-constructor): Implicit conversion to/from our `awaitable_type` is the point.

        ///@brief Default constructor.
        tagged_awaitable() = default;

        ///@brief Constructs from an awaitable.
        tagged_awaitable(awaitable_type awaitable) noexcept : awaitable_(std::move(awaitable)) {}

        ///@brief Implicit conversion to underlying awaitable (lvalue).
        operator awaitable_type&() noexcept { return awaitable_; }

        ///@brief Implicit conversion to underlying awaitable (const lvalue).
        operator const awaitable_type&() const noexcept { return awaitable_; }

        ///@brief Implicit conversion to underlying awaitable (rvalue).
        operator awaitable_type&&() noexcept { return std::move(awaitable_); }

        //NOLINTEND(google-explicit-constructor)

        ///@brief Supports co_await for lvalue.
        auto operator co_await() & noexcept { return awaitable_.operator co_await(); }

        ///@brief Supports co_await for const lvalue.
        auto operator co_await() const& noexcept { return awaitable_.operator co_await(); }

        ///@brief Supports co_await for rvalue.
        auto operator co_await() && noexcept { return std::move(awaitable_).operator co_await(); }
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
    using subnegotiation_awaitable =
        tagged_awaitable<tags::subnegotiation_tag, std::tuple<option, std::vector<byte_t>>>;
} //namespace net::telnet::awaitables
