// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file tools.test.coroutine_harness-dummies.cppm
 * @version 0.3.0
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
 * @brief Trivial awaitables for testing coroutine behavior.
 *
 * @details
 * Provides lightweight awaitables:
 * - `ready_awaiter`         = never suspends (always ready)
 * - `immediate_awaiter`     = suspends (never ready) then resumes immediately (by symmetric transfer)
 * - `adl::awaitable_by_adl` = awaitable with an ADL-only `operator co_await`
 *
 * Used to simulate coroutine interactions without external dependencies.
 *
 * @remark Depends on `:task` for promise interaction.
 */

//Module partition interface unit
export module tools.test.coroutine_harness:dummies;

import std; //NOLINT

import :task; ///< @see "tools.test.coroutine_harness-task.cppm"

export namespace tools::test::coroutine_harness::dummies {
    ///@brief Base class for trivial awaiters handling storage and value return from resume.
    template<typename T>
    struct trivial_awaiter_base {
    protected:
        // Only store value if T is not void
        using storage_t = std::conditional_t<std::is_void_v<T>, std::monostate, T>;
        storage_t storage_{};

    public:
        constexpr trivial_awaiter_base() = default;

        constexpr trivial_awaiter_base(storage_t val) noexcept(std::is_nothrow_move_constructible_v<storage_t>)
            requires (!std::is_void_v<T>)
            : storage_(std::move(val))
        {}

        constexpr auto await_resume() const& noexcept(std::is_nothrow_copy_constructible_v<T>)
            requires std::is_void_v<T> || std::is_copy_constructible_v<T>
        {
            if constexpr (std::is_void_v<T>) {
                return; // void optimization
            } else {
                return storage_;
            }
        }

        constexpr auto await_resume() & noexcept(std::is_nothrow_copy_constructible_v<T>)
            requires std::is_void_v<T> || std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>
        {
            if constexpr (std::is_void_v<T>) {
                return; // void optimization
            } else if constexpr (std::is_copy_constructible_v<T>) {
                return storage_;
            } else {
                return std::move(storage_);
            }
        }

        constexpr auto await_resume() && noexcept(std::is_nothrow_move_constructible_v<T>)
            requires std::is_void_v<T> || std::is_move_constructible_v<T>
        {
            if constexpr (std::is_void_v<T>) {
                return; // void optimization
            } else {
                return std::move(storage_);
            }
        }
    };

    ///@brief Trivial awaiter that is always ready.
    template<typename T>
    struct ready_awaiter : trivial_awaiter_base<T> {
        using base = trivial_awaiter_base<T>;
        using base::base;

        [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }

        [[noreturn]] std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const
        {
            throw std::logic_error("ready_awaiter had await_suspend called: contract violation");
        }
    };

    template<typename T>
    ready_awaiter(T) -> ready_awaiter<T>;

    ///@brief Trivial awaiter that suspends and immediately resumes via symmetric transfer.
    template<typename T>
    struct immediate_awaiter : trivial_awaiter_base<T> {
        using base = trivial_awaiter_base<T>;
        using base::base;

        [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

        template<typename U>
        [[nodiscard]] auto await_suspend(std::coroutine_handle<test_promise<U>> caller) noexcept
            -> std::coroutine_handle<test_promise<U>>
        {
            if (typename test_promise<U>::probe_ptr probe{caller.promise().probe}; probe)
                probe->suspended = true;
            return caller; // symmetric transfer → resume caller right away
        }

        [[nodiscard]] std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) const noexcept
        {
            return caller; // symmetric transfer → resume caller right away
        }
    };

    template<typename T>
    immediate_awaiter(T) -> immediate_awaiter<T>;

    namespace adl {
        ///@brief Dummy type made awaitable via ADL by free `operator co_await`
        template<typename T>
            requires (!std::is_void_v<T>)
        struct awaitable_by_adl {
            T value{};
        };

        ///@brief Free function `operator co_await` ADL hook for the dummy. @see `awaitable_by_adl`
        template<typename T>
        [[nodiscard]] auto operator co_await(awaitable_by_adl<T> dummy)
        {
            return ready_awaiter<T>{dummy.value};
        }
    } //namespace adl
} //namespace tools::test::coroutine_harness::dummies
