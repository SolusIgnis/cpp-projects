// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file tools.test.coroutine_harness-task.cppm
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
 * @brief Coroutine task and promise types.
 *
 * @details
 * Defines:
 * - `test_task<T>`        → coroutine handle wrapper
 * - `test_promise<T>`     → coroutine promise
 * - `test_promise_base<T>`
 *
 * Depends on `:core` for lifecycle tracking and handle management.
 *
 * @note A `test_task` may only be awaited once
 */

//Module partition interface unit
export module tools.test.coroutine_harness:task;

import std; //NOLINT

export import base.vocab.ptr; ///< for `alias_ptr`

import :core; ///< @see "tools.test.coroutine_harness-core.cppm"

export namespace tools::test::coroutine_harness {
    template<typename T>
    struct test_promise;

    /**
     * @brief Coroutine task wrapper used for deterministic testing.
     *
     * @tparam T Result type.
     *
     * @details
     * Provides:
     * - Safe coroutine lifetime management
     * - Double-await detection
     * - Probe instrumentation
     * - Exception propagation
     */
    template<typename T>
    class test_task {
    public:
        using promise_type = test_promise<T>;
        using probe_ptr    = promise_type::probe_ptr;

    private:
        template<typename HandleT>
            requires std::convertible_to<HandleT, std::coroutine_handle<promise_type>>
        struct awaiter {
            HandleT my_handle;

            [[nodiscard]] bool await_ready() const noexcept { return my_handle.done(); }

            template<typename U>
            [[nodiscard]] auto await_suspend(std::coroutine_handle<test_promise<U>> awaiting_handle) noexcept
                -> std::coroutine_handle<promise_type>
            {
                if (probe_ptr probe{awaiting_handle.promise().probe}; probe) {
                    probe->suspended = true;
                }
                my_handle.promise().continuation = awaiting_handle;
                return my_handle;
            }

            [[nodiscard]] auto await_suspend(std::coroutine_handle<> awaiting_handle) noexcept
                -> std::coroutine_handle<promise_type>
            {
                my_handle.promise().continuation = awaiting_handle;
                return my_handle;
            }

            [[nodiscard]] T await_resume()
            {
                auto& my_promise = my_handle.promise();
                if (my_promise.probe) {
                    my_promise.probe->resumed = true;
                }
                if (my_promise.exception) {
                    std::rethrow_exception(my_promise.exception);
                }

                if constexpr (std::same_as<T, void>) {
                    return;
                } else {
                    return std::move(*my_promise.value);
                }
            }
        };

    public:
        using aliasing_awaiter = awaiter<std::coroutine_handle<promise_type>>;
        using owning_awaiter   = awaiter<coroutine_handle_manager<T>>;

    private:
        coroutine_handle_manager<T> handle_;
        bool awaited_{false};

    public:
        test_task() = default;

        test_task(std::coroutine_handle<promise_type> h) : handle_(h) {}

        ~test_task() = default;

        test_task(const test_task&)            = delete;
        test_task& operator=(const test_task&) = delete;

        test_task(test_task&& other) noexcept
            : handle_(std::exchange(other.handle_, {})), awaited_(std::exchange(other.awaited_, {}))
        {
            if (handle_ && handle_.promise().probe)
                handle_.promise().probe->moved = true;
        }

        test_task& operator=(test_task&& other) noexcept(std::is_nothrow_swappable_v<test_task>)
        {
            if (this != &other) {
                swap(*this, other);

                if (handle_ && handle_.promise().probe)
                    handle_.promise().probe->moved = true;
            }
            return *this;
        }

        explicit operator bool() const noexcept { return static_cast<bool>(handle_); }

        test_task& set_probe(probe_ptr new_probe) &
        {
            do_set_probe(new_probe);
            return *this;
        }

        test_task&& set_probe(probe_ptr new_probe) &&
        {
            do_set_probe(new_probe);
            return std::move(*this);
        }

        [[nodiscard]] auto operator co_await() &
        {
            prepare_co_await(coroutine_probe::path::lvalue);
            return aliasing_awaiter{handle_.get()};
        }

        [[nodiscard]] auto operator co_await() &&
        {
            prepare_co_await(coroutine_probe::path::rvalue);
            return owning_awaiter{std::exchange(handle_, {})};
        }

        friend void swap(test_task& lhs, test_task& rhs) noexcept(std::is_nothrow_swappable_v<coroutine_handle_manager<T>>)
        {
            using std::swap;
            swap(lhs.handle_, rhs.handle_);
            swap(lhs.awaited_, rhs.awaited_);
        }

    private:
        void do_set_probe(probe_ptr new_probe)
        {
            if (handle_)
                handle_.promise().probe = new_probe;
        }

        void prepare_co_await(coroutine_probe::path await_path)
        {
            if (!handle_) {
                throw std::logic_error("test_task awaited while empty");
            }
            if (awaited_) {
                throw std::logic_error("test_task awaited more than once");
            }
            awaited_ = true;

            if (probe_ptr probe{handle_.promise().probe}; probe) {
                probe->awaited    = true;
                probe->await_path = await_path;
            }
        }
    };

    /// @brief Shared base for promise implementations.
    template<typename T>
    struct test_promise_base {
        using storage_t = std::conditional_t<std::is_void_v<T>, std::monostate, T>;
        using probe_ptr = base::vocab::alias_ptr<coroutine_probe>;

        std::optional<storage_t> value{};
        std::exception_ptr exception{};
        probe_ptr probe{nullptr};

        std::coroutine_handle<> continuation{std::noop_coroutine()};

        test_task<T> get_return_object(this auto& self)
        {
            return test_task<T>{std::coroutine_handle<std::remove_reference_t<decltype(self)>>::from_promise(self)};
        }

        struct suspend_finalize {
            [[nodiscard]] bool await_ready() const noexcept { return false; }

            [[nodiscard]] auto await_suspend(std::coroutine_handle<test_promise<T>> finalizing_handle) noexcept
            {
                auto& finalizing_promise = finalizing_handle.promise();
                if (probe_ptr probe{finalizing_promise.probe}; probe) {
                    probe->done = true;
                }
                return finalizing_promise.continuation;
            }

            void await_resume() noexcept {}
        };

        auto initial_suspend() noexcept { return std::suspend_always{}; }

        auto final_suspend() noexcept { return suspend_finalize{}; }

        void unhandled_exception() noexcept { exception = std::current_exception(); }
    };

    /// @brief Promise for non-void coroutine results.
    template<typename T>
    struct test_promise : test_promise_base<T> {
        void return_value(T v) noexcept
        try {
            this->value.emplace(std::move(v));
        } catch (...) {
            this->unhandled_exception();
        }
    };

    /// @brief Promise specialization for `void`.
    template<>
    struct test_promise<void> : test_promise_base<void> {
        void return_void() noexcept
        try {
            this->value.emplace();
        } catch (...) {
            this->unhandled_exception();
        }
    };
} //namespace tools::test::coroutine_harness
