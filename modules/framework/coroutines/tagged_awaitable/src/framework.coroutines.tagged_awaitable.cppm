// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @module framework.coroutines.tagged_awaitable
 * @file framework.coroutines.tagged_awaitable.cppm
 * @version 0.2.0
 * @date April 12, 2026
 *
 * @copyright © 2026 Jeremy Murphy and any Contributors
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
 * @brief Primary module interface for the `tagged_awaitable` library
 * Defines `tagged_awaitable` class to wrap awaitable types with semantic tags.
 */

//Primary module interface unit
export module framework.coroutines.tagged_awaitable;

import std;

namespace framework::coroutines {
    struct adl_lookup_tag {};

    //Delete free `operator co_await` for ADL purposes.
    void operator co_await(adl_lookup_tag) = delete;
} //namespace framework::coroutines

export namespace framework::coroutines {
    /**
     * @brief Wrapper for an awaitable with a semantic tag for type safety.
     * @tparam Tag The semantic tag type.
     * @tparam AwaitableT The underlying awaitable type.
     * @remark Provides implicit conversion to/from the underlying awaitable and supports direct `co_await`.
     */
    template<typename Tag, typename AwaitableT>
    class tagged_awaitable {
    private:
        using awaitable_type = AwaitableT; ///< Underlying awaitable type

        awaitable_type awaitable_; ///< The wrapped awaitable

    public:
        //NOLINTBEGIN(google-explicit-constructor): Implicit conversion to/from our `awaitable_type` is the point.

        ///@brief Default constructor.
        tagged_awaitable() = default;

        ///@brief Constructs from an awaitable.
        explicit(false) tagged_awaitable(
            awaitable_type awaitable
        ) noexcept(std::is_nothrow_move_constructible_v<awaitable_type>)
            : awaitable_(std::move(awaitable))
        {}

        ///@brief Prevent copy construction from a tagged_awaitable with a different tag.
        template<typename OtherTag, typename OtherAwaitable>
        tagged_awaitable(const tagged_awaitable<OtherTag, OtherAwaitable>&) = delete;

        ///@brief Prevent move construction from a tagged_awaitable with a different tag.
        template<typename OtherTag, typename OtherAwaitable>
        tagged_awaitable(tagged_awaitable<OtherTag, OtherAwaitable>&&) = delete;

        ///@brief Implicit conversion to underlying awaitable (lvalue).
        explicit(false) operator awaitable_type() & noexcept(std::is_nothrow_constructible_v<awaitable_type, awaitable_type&>)
        {
            return awaitable_;
        }

        ///@brief Implicit conversion to underlying awaitable (const lvalue).
        explicit(false) operator awaitable_type() const& noexcept(std::is_nothrow_copy_constructible_v<awaitable_type>)
        {
            return awaitable_;
        }

        ///@brief Implicit conversion to underlying awaitable (rvalue).
        explicit(false) operator awaitable_type() && noexcept(std::is_nothrow_move_constructible_v<awaitable_type>)
        {
            return std::move(awaitable_);
        }

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
        template<typename Self>
            requires std::same_as<std::remove_cvref_t<Self>, tagged_awaitable>
        friend decltype(auto) operator co_await(Self&& wrapper)
            requires (!requires { std::forward<Self>(wrapper).awaitable_.operator co_await(); })
        {
            using framework::coroutines::operator co_await;
            auto&& awaitable = std::forward<Self>(wrapper).awaitable_;

            if constexpr (requires { operator co_await(std::forward<decltype(awaitable)>(awaitable)); }) {
                return operator co_await(std::forward<decltype(awaitable)>(awaitable));
            } else {
                return std::forward<decltype(awaitable)>(awaitable);
            }
        }
    }; //class tagged_awaitable

    /**
     * @fn tagged_awaitable::tagged_awaitable(awaitable_type awaitable) noexcept
     * @param awaitable The awaitable to wrap.
     * @note Implicit conversion from the underlying type allows direct returns from e.g. Asio asynchronous operations.
     */
} //namespace framework::coroutines

namespace std {
    ///@brief Partial specialization of `std::coroutine_traits` forwarding the promise type for a `tagged_awaitable` to the promise type of its underlying awaitable type.
    template<typename Tag, typename AwaitableT, typename... Args>
    struct coroutine_traits<framework::coroutines::tagged_awaitable<Tag, AwaitableT>, Args...> {
        using promise_type = std::coroutine_traits<AwaitableT, Args...>::promise_type;
    };
} //namespace std
