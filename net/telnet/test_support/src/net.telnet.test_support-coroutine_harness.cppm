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

    template<typename T>
    class coroutine_handle_manager {
        using promise_type    = test_promise<T>;
        using probe_ptr       = promise_type::probe_ptr;
        using raw_handle_type = std::coroutine_handle<promise_type>;

        raw_handle_type handle_;

    public:
        coroutine_handle_manager() = default;

        explicit coroutine_handle_manager(raw_handle_type handle) : handle_(handle) {}

        ~coroutine_handle_manager() noexcept(false) { destroy(); }

        coroutine_handle_manager(const coroutine_handle_manager&)            = delete;
        coroutine_handle_manager& operator=(const coroutine_handle_manager&) = delete;

        coroutine_handle_manager(coroutine_handle_manager&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}

        coroutine_handle_manager& operator=(coroutine_handle_manager&& other) noexcept(false)
        {
            if (this != &other) {
                if (handle_)
                    destroy();

                handle_ = std::exchange(other.handle_, {});
            }
            return *this;
        }

        raw_handle_type& get() { return handle_; }

        const raw_handle_type& get() const { return handle_; }

        explicit(false) operator raw_handle_type() { return handle_; }

        explicit operator bool() const noexcept { return handle_ != nullptr; }

        bool done() const { return handle_.done(); }

        decltype(auto) promise() { return handle_.promise(); }

    private:
        void destroy() noexcept(false)
        {
            if (handle_) {
                auto destroying_handle = std::exchange(handle_, {});

                bool done{destroying_handle.done()};
                probe_ptr probe{destroying_handle.promise().probe};

                destroying_handle.destroy();

                if (probe) {
                    probe->destroyed = true;
                } else if (!done && (std::uncaught_exceptions() == 0)) {
                    throw std::logic_error(
                        "test_task coroutine frame destroyed before coroutine completion without probe attached"
                    );
                }
            }
        }
    };

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
                if (probe_ptr probe{awaiting_handle.promise().probe}; probe)
                    probe->suspended = true;
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

        test_task(test_task&& other) noexcept : handle_(std::exchange(other.handle_, {}))
        {
            if (handle_ && handle_.promise().probe)
                handle_.promise().probe->moved = true;
        }

        test_task& operator=(test_task&& other)
        {
            if (this != &other) {
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
            return aliasing_awaiter{handle_.get()};
        }

        [[nodiscard]] auto operator co_await() &&
        {
            prepare_co_await(coroutine_probe::path::rvalue);
            return owning_awaiter{std::exchange(handle_, {})};
        }

    private:
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

            if (probe_ptr probe{handle_.promise().probe}; probe) {
                probe->awaited    = true;
                probe->await_path = await_path;
            }
        }
    };

    template<typename T, typename Derived>
    struct test_promise_base {
        using storage_t = std::conditional_t<std::is_void_v<T>, std::monostate, T>;
        using probe_ptr = base::vocab::ptr::alias_ptr<coroutine_probe>;

        std::optional<storage_t> value{};
        std::exception_ptr exception{};
        probe_ptr probe{nullptr};

        std::coroutine_handle<> continuation{std::noop_coroutine()};

        test_task<T> get_return_object()
        {
            return test_task<T>{std::coroutine_handle<Derived>::from_promise(static_cast<Derived&>(*this))};
        }

        struct suspend_finalize {
            [[nodiscard]] bool await_ready() const noexcept { return false; }

            [[nodiscard]] auto await_suspend(std::coroutine_handle<Derived> finalizing_handle) noexcept
            {
                auto& finalizing_promise = finalizing_handle.promise();
                if (probe_ptr probe{finalizing_promise.probe}; probe) {
                    probe->done = true;
                }
                return finalizing_promise.continuation;
            }

            void await_resume() noexcept {}
        };

        std::suspend_always initial_suspend() noexcept { return {}; }

        suspend_finalize final_suspend() noexcept { return {}; }

        void unhandled_exception() noexcept { exception = std::current_exception(); }
    };

    template<typename T>
    struct test_promise : test_promise_base<T, test_promise<T>> {
        void return_value(T v) noexcept { this->value.emplace(std::move(v)); }
    };

    template<>
    struct test_promise<void> : test_promise_base<void, test_promise<void>> {
        void return_void() noexcept { this->value.emplace(); }
    };

    namespace dummies {
        ///@brief Base class for trivial awaiters handling storage and value return from resume.
        template<typename T>
        struct trivial_awaiter_base {
        protected:
            // Only store value if T is not void
            using storage_t = std::conditional_t<std::is_void_v<T>, std::monostate, T>;
            storage_t storage_{};
        
        public:
            constexpr trivial_awaiter_base() = default;
            constexpr trivial_awaiter_base(storage_t val) noexcept(std::is_nothrow_move_constructible_v<storage_t>) requires (!std::is_void_v<T>)
                : storage_(std::move(val)) {}
        
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
        
            [[noreturn]] std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const {
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
        
            [[nodiscard]] std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) const noexcept {
                return caller; // symmetric transfer → resume caller right away
            }
        };
        
        template<typename T>
        immediate_awaiter(T) -> immediate_awaiter<T>;
    
        namespace adl {
            ///@brief Dummy type made awaitable by free `operator co_await`
            template<typename T> requires (!std::is_void_v<T>)
            struct awaitable_by_adl {
                T value{};
            };
        
            ///@brief `operator co_await` for the dummy. @see `awaitable_by_adl`
            template<typename T>
            [[nodiscard]] auto operator co_await(awaitable_by_adl<T> dummy)
            {
                return ready_awaiter<T>{dummy.value};
            }
        } //namespace adl
    } //namespace dummies

    ///@brief Wrap a trivial awaiter or non-coroutine awaitable in a `test_task` so that it can be run as a task.
    template<typename T, typename Awaitable>
    [[nodiscard]] auto as_task(Awaitable&& awaitable) -> test_task<T>
    {
        std::shared_ptr<std::remove_reference_t<Awaitable>> stored{std::make_unique(std::forward<Awaitable>(awaitable))};
        co_return co_await *stored;
    }

    template<typename Task>
    auto test_runner_entry(Task&& task) -> std::remove_reference_t<Task>
    {
        co_return co_await std::forward<Task>(task);
    }

    template<typename Task>
    [[nodiscard]] decltype(auto) run(Task&& task)
    {
        auto entry_point{test_runner_entry(std::forward<Task>(task))};

        auto awaiter{std::forward<Task>(entry_point).operator co_await()};

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
