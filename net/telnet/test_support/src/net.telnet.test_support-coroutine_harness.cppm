// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file net.telnet.test_suport-coroutine_harness.cppm
 * @version 0.1.0
 * @date March 9, 2026
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
 * @brief Coroutine test harness with synchronous runner.
 *
 * This module provides `test_task` and `test_promise` types for unit-testing coroutines
 * in a synchronous, deterministic fashion. It tracks coroutine lifecycle events via
 * `coroutine_probe` and ensures safe destruction, double-await detection, and exception propagation.
 *
 * @remark Detects double-awaits and moves.
 * @remark Tracks suspension, resumption, and destruction via `coroutine_probe`.
 * @remark Supports `void` and non-`void` coroutine return types.
 * @remark Provides a synchronous runner `run()` that executes coroutines to completion or throws if the coroutine stalls or propagates exceptions.
 *
 * @example @parblock
 * ## Example Usage
 * @code ```cpp
 * using namespace net::telnet::test_support;
 *
 * coroutine_probe probe;
 *
 * // Create a test coroutine task that returns 42
 * auto my_task = []() -> test_task<int>
 * {
 *     co_return 42;
 * }();
 *
 * my_task.set_probe(&probe);
 *
 * // Run and await the coroutine within this scope
 * {
 *     int result = run(my_task);
 *
 *     using enum coroutine_probe::path;
 *     assert(result == 42);
 *     assert(probe.awaited);    // co_await was called
 *     assert(probe.resumed);    // coroutine resumed after suspension
 *     assert(!probe.moved);     // task was not moved (lvalue reference passed to run)
 *     assert(probe.done);       // final_suspend reached
 *     assert(!probe.destroyed); // coroutine frame not destroyed yet
 *     assert(probe.await_path == lvalue); // lvalue co_await was called
 * }
 *
 * // After leaving the scope, the coroutine frame is destroyed
 * assert(probe.destroyed);
 * ``` @endcode @endparblock
 *
 * @remark `operator co_await()` distinguishes lvalue vs rvalue `test_task` to track await paths.
 * @remark `test_runner_entry` and `run()` ensure the coroutine completes synchronously, with any exceptions propagated.
 */

//Module partition interface unit
export module net.telnet.test_support:coroutine_harness;

import std;

import base.vocab.ptr;

export namespace net::telnet::test_support::coroutine_harness {
    //Forward declaration of promise type used by task type but defined later
    template<typename T>
    struct test_promise;

    struct coroutine_probe {
        enum class path : std::uint8_t {
            none,
            lvalue,
            rvalue
        };

        bool done{false};
        bool destroyed{false};
        bool awaited{false};
        bool suspended{false};
        bool resumed{false};
        bool moved{false};
        path await_path{path::none};
    };

    template<typename T, typename PromiseT>
    struct test_awaiter {
        using promise_type = PromiseT;
        using probe_ptr    = promise_type::probe_ptr;

        std::coroutine_handle<promise_type> my_handle;
        bool ownership;

        ~test_awaiter() noexcept(false) { destroy(); }

        test_awaiter(const test_awaiter&)            = delete;
        test_awaiter& operator=(const test_awaiter&) = delete;

        test_awaiter(test_awaiter&& other) noexcept
            : my_handle(std::exchange(other.my_handle, {})), ownership(std::exchange(other.ownership, false))
        {}

        test_awaiter& operator=(test_awaiter&& other) noexcept(false)
        {
            if (this != &other) {
                if (ownership && my_handle)
                    destroy();

                my_handle = std::exchange(other.my_handle, {});
                ownership = std::exchange(other.ownership, false);
            }
            return *this;
        }

        [[nodiscard]] bool await_ready() noexcept { return my_handle.done(); }

        [[nodiscard]] auto await_suspend(std::coroutine_handle<promise_type> awaiting_handle) noexcept
        {
            if (probe_ptr probe = awaiting_handle.promise().probe; probe)
                probe->suspended = true;
            my_handle.promise().continuation = awaiting_handle;
            return my_handle;
        }

        [[nodiscard]] auto await_suspend(std::coroutine_handle<> awaiting_handle) noexcept
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

    private:
        void destroy() noexcept(false)
        {
            if (ownership && my_handle) {
                bool done       = my_handle.done();
                probe_ptr probe = my_handle.promise().probe;

                my_handle.destroy();

                if (probe) {
                    probe->destroyed = true;
                } else if (!done && (std::uncaught_exceptions() == 0)) {
                    throw std::logic_error("test_task destroyed before coroutine completion without probe attached");
                }
            }
        }
    };

    template<typename T, typename PromiseT = test_promise<T>, typename AwaiterT = test_awaiter<T, PromiseT>>
    class test_task {
    public:
        using promise_type = PromiseT;
        using probe_ptr    = promise_type::probe_ptr;

    private:
        std::coroutine_handle<promise_type> handle_;
        bool awaited_{false};

    public:
        test_task(std::coroutine_handle<promise_type> h) : handle_(h) {}

        ~test_task() noexcept(false) { destroy(); }

        test_task(const test_task&)            = delete;
        test_task& operator=(const test_task&) = delete;

        test_task(test_task&& other) noexcept : handle_(std::exchange(other.handle_, {}))
        {
            if (handle_ && handle_.promise().probe)
                handle_.promise().probe->moved = true;
        }

        test_task& operator=(test_task&& other) noexcept(false)
        {
            if (this != &other) {
                if (handle_)
                    destroy();

                handle_ = std::exchange(other.handle_, {});

                if (handle_ && handle_.promise().probe)
                    handle_.promise().probe->moved = true;
            }
            return *this;
        }

        explicit operator bool() const noexcept { return handle_ != nullptr; }

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
            return awaiter{handle_, false};
        }

        [[nodiscard]] auto operator co_await() &&
        {
            prepare_co_await(coroutine_probe::path::rvalue);
            return awaiter{std::exchange(handle_, {}), true};
        }

    private:
        using awaiter = AwaiterT;

        void do_set_probe(probe_ptr new_probe)
        {
            if (handle_)
                handle_.promise().probe = new_probe;
        }

        void prepare_co_await(coroutine_probe::path await_path)
        {
            if (awaited_) {
                throw std::logic_error("test_task awaited more than once");
            }
            awaited_ = true;

            if (probe_ptr probe = handle_.promise().probe; probe) {
                probe->awaited    = true;
                probe->await_path = await_path;
            }
        }

        void destroy() noexcept(false)
        {
            if (handle_) {
                bool done       = handle_.done();
                probe_ptr probe = handle_.promise().probe;

                handle_.destroy();

                if (probe) {
                    probe->destroyed = true;
                } else if (!done && (std::uncaught_exceptions() == 0)) {
                    throw std::logic_error("test_task destroyed before coroutine completion without probe attached");
                }
            }
        }
    };

    template<typename T>
    struct test_promise {
        using storage_t = std::conditional_t<std::same_as<T, void>, std::monostate, T>;
        using probe_ptr = base::vocab::ptr::alias_ptr<coroutine_probe>;

        std::optional<storage_t> value{};
        std::exception_ptr exception{};
        probe_ptr probe{nullptr};

        std::coroutine_handle<> continuation = std::noop_coroutine();

        test_task<T> get_return_object() { return test_task<T>{std::coroutine_handle<test_promise>::from_promise(*this)}; }

        struct suspend_finalize {
            [[nodiscard]] bool await_ready() noexcept { return false; }

            [[nodiscard]] auto await_suspend(std::coroutine_handle<test_promise> finalizing_handle) noexcept
            {
                auto& finalizing_promise = finalizing_handle.promise();
                if (probe_ptr probe = finalizing_promise.probe; probe) {
                    probe->done = true;
                }
                return finalizing_promise.continuation;
            }

            void await_resume() noexcept {}
        };

        std::suspend_always initial_suspend() noexcept { return {}; }

        suspend_finalize final_suspend() noexcept { return {}; }

        void return_value(T v) noexcept
            requires (!std::same_as<T, void>)
        {
            value.emplace(std::move(v));
        }

        void return_void() noexcept
            requires std::same_as<T, void>
        {
            value.emplace();
        }

        void unhandled_exception() noexcept { exception = std::current_exception(); }
    };

    template<typename Task>
    Task test_runner_entry(Task&& task)
    {
        co_return co_await task;
    }

    template<typename Task>
    [[nodiscard]] decltype(auto) run(Task&& task)
    {
        auto entry_point = test_runner_entry(std::forward<Task>(task));

        auto awaiter = entry_point.operator co_await();

        if (!awaiter.await_ready()) {
            (awaiter.await_suspend(std::noop_coroutine())).resume();
        }

        if (awaiter.my_handle.done()) {
            return awaiter.await_resume();
        } else {
            throw std::system_error(
                std::make_error_code(std::errc::resource_unavailable_try_again),
                "Coroutine failed to complete (stalled at suspension point)"
            );
        }
    }
} //namespace net::telnet::test_support::coroutine_harness
