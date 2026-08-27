// SPDX-License-Identifier: Apache-2.0
// Unit tests for tools.test.coroutine_harness:runner

import tools.test.coroutine_harness;

import ut;
import std;

using namespace ut;
using namespace tools::test::coroutine_harness;

test_task<int> echo(int value)
{
    co_return value;
}

suite as_task_adapter_tests = [] mutable {
    // ============================================================
    // as_task adapter
    // ============================================================

    "as_task forwards value (ready path)"_test = [] mutable {
        // Bespoke trivial awaiter for bootstrapping tests. as_task tests can't depend on the test dummies namespace since their tests depend on as_task.
        struct echo_ready_awaiter {
            int value{};
            [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }
            constexpr void await_suspend(std::coroutine_handle<> /*unused*/) const noexcept {}
            [[nodiscard]] constexpr int await_resume() const { return value; }
        };

        constexpr int expected = 55;
        const auto result      = run(as_task<int>(echo_ready_awaiter{expected}));
        expect(eq(result, expected));
    };

    "as_task preserves suspension semantics"_test = [] mutable {
        // Bespoke trivial awaiter for bootstrapping tests. as_task tests can't depend on the test dummies namespace since their tests depend on as_task.
        struct echo_immediate_awaiter {
            int value{};
            [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }
            [[nodiscard]] auto await_suspend(std::coroutine_handle<test_promise<int>> caller) noexcept
            {
                if (test_promise<int>::probe_ptr probe{caller.promise().probe}; probe) {
                    probe->suspended = true;
                }
                return caller; // symmetric transfer → resume caller right away
            }
            [[nodiscard]] constexpr int await_resume() const { return value; }
        };

        constexpr int expected = 10;

        coroutine_probe probe;

        auto task = as_task<int>(echo_immediate_awaiter{expected});
        task.set_probe(&probe);

        const auto result = run(task);

        expect(eq(result, expected));
        expect(eq(probe.suspended, true));
        expect(eq(probe.resumed, true));
    };

    "as_task propagates exception from await_resume"_test = [] mutable {
        struct throwing_awaiter {
            [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }
            constexpr void await_suspend(std::coroutine_handle<> /*unused*/) const noexcept {}
            [[noreturn]] int await_resume() { throw std::runtime_error("boom"); }
        };

        bool threw = false;

        try {
            [[maybe_unused]] auto result = run(as_task<int>(throwing_awaiter{}));
        } catch (const std::runtime_error&) {
            threw = true;
        }

        expect(eq(threw, true));
    };

    "as_task propagates exception from await_suspend"_test = [] mutable {
        struct throwing_suspend_awaiter {
            [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }
            [[noreturn]] void await_suspend(std::coroutine_handle<> /*unused*/) { throw std::runtime_error("boom"); }
            constexpr void await_resume() const noexcept { return; }
        };

        bool threw = false;
        try {
            run(as_task<void>(throwing_suspend_awaiter{}));
        } catch (const std::runtime_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };

    "as_task supports move-only return types (unique_ptr)"_test = [] mutable {
        // Bespoke trivial awaiter for bootstrapping tests. as_task tests can't depend on the test dummies namespace since their tests depend on as_task.
        struct echo_ready_awaiter {
            std::unique_ptr<int> value;
            [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }
            constexpr void await_suspend(std::coroutine_handle<> /*unused*/) const noexcept {}
            [[nodiscard]] constexpr std::unique_ptr<int> await_resume() { return std::move(value); }
        };

        constexpr int expected = 42;

        // Create an awaiter that holds a move-only type
        auto awaiter = echo_ready_awaiter{std::make_unique<int>(expected)};

        // Wrap it. This requires perfect forwarding to work inside 'as_task'
        auto task = as_task<std::unique_ptr<int>>(std::move(awaiter));

        // Run it. This requires the promise_type to correctly move the value out.
        const std::unique_ptr<int> result = run(std::move(task));

        expect(eq(static_cast<bool>(result), true));
        if (result) {
            expect(eq(*result, expected));
        }
    };

    "as_task supports wrapping a test_task"_test = [] mutable {
        int expected = 42;
        auto result  = run(as_task<int>(echo(expected)));

        expect(eq(result, expected));
    };

    "as_task supports nested calls"_test = [] mutable {
        // Bespoke trivial awaiter for bootstrapping tests. as_task tests can't depend on the test dummies namespace since their tests depend on as_task.
        struct echo_ready_awaiter {
            int value{};
            [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }
            constexpr void await_suspend(std::coroutine_handle<> /*unused*/) const noexcept {}
            [[nodiscard]] constexpr int await_resume() const { return value; }
        };
        int expected = 42;

        auto taskA = as_task<int>(echo_ready_awaiter{expected});
        auto taskB = as_task<int>(as_task<int>(std::move(taskA)));

        auto result = run(taskB);
        expect(eq(result, expected));
    };
};

int main() {}
