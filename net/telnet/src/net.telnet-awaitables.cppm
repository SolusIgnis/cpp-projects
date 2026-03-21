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

import std; //NOLINT

export import :options; ///< @see "net.telnet-options.cppm" for `option`

namespace net::telnet::awaitables {
    struct adl_lookup_tag {};

    //Delete free `operator co_await` for ADL purposes.
    void operator co_await(adl_lookup_tag) = delete;

    /**
     * @brief Wrapper for an awaitable with a semantic tag for type safety.
     * @tparam Tag The semantic tag type.
     * @tparam AwaitableT The underlying `Awaitable` type (eg: `asio::awaitable<void>`).
     * @remark Provides implicit conversion to/from the underlying awaitable and supports direct `co_await`.
     * @see `tags` namespace for semantic tag types, `:protocol_fsm`, `:internal`
     */
    template<typename Tag, typename AwaitableT>
    class tagged_awaitable_impl {
    private:
        using awaitable_type = AwaitableT; ///< Underlying awaitable type

        awaitable_type awaitable_; ///< The wrapped awaitable

    public:
        //NOLINTBEGIN(google-explicit-constructor): Implicit conversion to/from our `awaitable_type` is the point.

        ///@brief Default constructor.
        tagged_awaitable_impl() = default;

        ///@brief Constructs from an awaitable.
        explicit(false) tagged_awaitable_impl(awaitable_type awaitable) noexcept : awaitable_(std::move(awaitable)) {}

        ///@brief Prevent copy construction from a tagged_awaitable with a different tag.
        template<typename OtherTag, typename OtherAwaitable>
        tagged_awaitable_impl(tagged_awaitable_impl<OtherTag, OtherAwaitable> const&) = delete;

        ///@brief Prevent move construction from a tagged_awaitable with a different tag.
        template<typename OtherTag, typename OtherAwaitable>
        tagged_awaitable_impl(tagged_awaitable_impl<OtherTag, OtherAwaitable>&&) = delete;

        ///@brief Implicit conversion to underlying awaitable (lvalue).
        explicit(false) operator awaitable_type() & noexcept { return awaitable_; }

        ///@brief Implicit conversion to underlying awaitable (const lvalue).
        explicit(false) operator awaitable_type() const& noexcept { return awaitable_; }

        ///@brief Implicit conversion to underlying awaitable (rvalue).
        explicit(false) operator awaitable_type() && noexcept { return std::move(awaitable_); }

        //NOLINTEND(google-explicit-constructor)

        ///@brief Explicit conversion to underlying awaitable (lvalue).
        awaitable_type& get() & noexcept { return awaitable_; }

        ///@brief Explicit conversion to underlying awaitable (const lvalue).
        const awaitable_type& get() const& noexcept { return awaitable_; }

        ///@brief Explicit conversion to underlying awaitable (rvalue).
        awaitable_type&& get() && noexcept { return std::move(awaitable_); }

        ///@brief Supports member `co_await` for lvalue.
        decltype(auto) operator co_await() &
            requires requires { awaitable_.operator co_await(); }
        {
            return awaitable_.operator co_await();
        }

        ///@brief Supports member `co_await` for const lvalue.
        decltype(auto) operator co_await() const&
            requires requires { awaitable_.operator co_await(); }
        {
            return awaitable_.operator co_await();
        }

        ///@brief Supports member `co_await` for rvalue.
        decltype(auto) operator co_await() &&
            requires requires { std::move(awaitable_).operator co_await(); }
        {
            return std::move(awaitable_).operator co_await();
        }

        ///@brief Supports ADL `co_await` universally.
        template <typename Self>
            requires std::same_as<std::remove_cvref_t<Self>, tagged_awaitable_impl>
        friend decltype(auto) operator co_await(Self&& wrapper)
            requires (!requires { std::forward<Self>(wrapper).awaitable_.operator co_await(); })
        {
            using net::telnet::awaitables::operator co_await;
            auto&& awaitable = std::forward<Self>(wrapper).awaitable_;
            
            if constexpr (requires { operator co_await(std::forward<decltype(awaitable)>(awaitable)); }) {
                return operator co_await(std::forward<decltype(awaitable)>(awaitable));
            } else {
                return std::forward<decltype(awaitable)>(awaitable);
            }
        }
#if 0
        ///@brief Supports ADL co_await for lvalue.
        friend decltype(auto) operator co_await(tagged_awaitable_impl& wrapper)
            requires (!requires { wrapper.awaitable_.operator co_await(); })
        {
            using net::telnet::awaitables::operator co_await;
            if constexpr (requires { operator co_await(wrapper.awaitable_); }) {
                return operator co_await(wrapper.awaitable_);
            } else {
                return (wrapper.awaitable_);
            }
        }

        ///@brief Supports ADL co_await for const lvalue.
        friend decltype(auto) operator co_await(const tagged_awaitable_impl& wrapper)
            requires (!requires { wrapper.awaitable_.operator co_await(); })
        {
            using net::telnet::awaitables::operator co_await;
            if constexpr (requires { operator co_await(wrapper.awaitable_); }) {
                return operator co_await(wrapper.awaitable_);
            } else {
                return (wrapper.awaitable_);
            }
        }

        ///@brief Supports ADL co_await for rvalue.
        friend decltype(auto) operator co_await(tagged_awaitable_impl&& wrapper)
            requires (!requires { std::move(wrapper.awaitable_).operator co_await(); })
        {
            using net::telnet::awaitables::operator co_await;
            if constexpr (requires { operator co_await(std::move(wrapper.awaitable_)); }) {
                return operator co_await(std::move(wrapper.awaitable_));
            } else {
                return std::move(wrapper.awaitable_);
            }
        }
#endif
    }; //class tagged_awaitable_impl

    /**
     * @fn tagged_awaitable_impl::tagged_awaitable_impl(awaitable_type awaitable) noexcept
     * @param awaitable The awaitable to wrap.
     * @note Implicit conversion from the underlying type allows direct returns from Boost.Asio asynchronous operations.
     */
     
    // 1. The "Catch-All" Primary
    template<typename Tag, typename T, typename... Extra>
    class tagged_awaitable_selector;

    // 2. Specialization A: You passed a Type (e.g., asio::awaitable<int>)
    // Usage: tagged_awaitable<my_tag, asio::awaitable<int>>
    template<typename Tag, typename FullType>
    class tagged_awaitable_selector<Tag, FullType> {
        using type = tagged_awaitable_impl<Tag, FullType>;
    };

    // 3. Specialization B: You passed a Template + Args
    // Usage: tagged_awaitable<my_tag, asio::awaitable, int>
    template<typename Tag, template<typename...> typename Templ, typename... Args>
    class tagged_awaitable_selector<Tag, Templ, Args...> {
        using type = tagged_awaitable_impl<Tag, Templ<Args...>>;
    };
} //namespace net::telnet::awaitables
 
export namespace net::telnet::awaitables {
    
    // 4. The Exported Alias
    template<typename Tag, typename... Args>
    using tagged_awaitable = typename tagged_awaitable_selector<Tag, Args...>::type;

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
    using option_enablement_awaitable = tagged_awaitable<tags::option_enablement_tag, asio::awaitable, void>;

    /**
     * @typedef option_disablement_awaitable
     * @brief Awaitable type for option disablement handlers.
     * @see `tagged_awaitable`, `tags::option_disablement_tag`, `:internal` (`option_handler_registry`), `:protocol_fsm` (for use)
     */
    using option_disablement_awaitable = tagged_awaitable<tags::option_disablement_tag, asio::awaitable, void>;

    /**
     * @typedef subnegotiation_awaitable
     * @brief Awaitable type for subnegotiation handlers.
     * @see `tagged_awaitable`, `tags::subnegotiation_tag`, `:internal` (`option_handler_registry`), `:protocol_fsm` (for use)
     */
    using subnegotiation_awaitable = tagged_awaitable<tags::subnegotiation_tag, asio::awaitable, std::tuple<option, std::vector<byte_t>>>;
} //namespace net::telnet::awaitables

namespace std {
    ///@brief Partial specialization of `std::coroutine_traits` forwarding the promise type for a `tagged_awaitable` to the promise type of its underlying awaitable type.
    template<typename Tag, typename AwaitableT, typename... Args>
    struct coroutine_traits<
        net::telnet::awaitables::tagged_awaitable_impl<Tag, AwaitableT>,
        Args...
    > {
        using promise_type = typename std::coroutine_traits<AwaitableT, Args...>::promise_type;
    };
} //namespace std