// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet.test_support:coroutine_harness

import net.telnet.test_support;
import base.vocab;
import ut;
import std;

using namespace ut;
using namespace net::telnet::test_support::coroutine_harness;

test_task<int> echo(int value)
{
    co_return value;
}

test_task<test_task<int>> make_echo(int value, base::vocab::ptr::alias_ptr<coroutine_probe> probe = nullptr)
{
    co_return echo(value).set_probe(probe);
}

suite coroutine_harness_tests = [] mutable {
    "run returns value"_test = [] mutable {
        auto task = echo(42);

        auto result = run(task);

        expect(eq(result, 42));
    };

    "probe initialization"_test = [] mutable {
        coroutine_probe probe;

        expect(eq(probe.done, false));
        expect(eq(probe.destroyed, false));
        expect(eq(probe.awaited, false));
        expect(eq(probe.suspended, false));
        expect(eq(probe.resumed, false));
        expect(eq(probe.moved, false));
        expect(eq(static_cast<int>(probe.await_path), static_cast<int>(coroutine_probe::path::none)));
    };

    "run returns void"_test = [] mutable {
        coroutine_probe probe;

        auto task = []() -> test_task<void> { co_return; }();

        task.set_probe(&probe);

        run(task);

        expect(eq(probe.awaited, true));
        expect(eq(probe.done, true));
    };

    "probe lifecycle"_test = [] mutable {
        coroutine_probe probe;

        {
            auto task = echo(42);

            task.set_probe(&probe);

            [[maybe_unused]] auto result = run(task);

            expect(eq(probe.awaited, true));
            expect(eq(probe.suspended, false));
            expect(eq(probe.resumed, true));
            expect(eq(probe.done, true));
            expect(eq(probe.destroyed, false));
            expect(eq(probe.moved, false));
            expect(eq(static_cast<int>(probe.await_path), static_cast<int>(coroutine_probe::path::lvalue)));
        }

        expect(eq(probe.destroyed, true));
    };

    "rvalue await path"_test = [] mutable {
        coroutine_probe probe;

        auto task = echo(42);
        task.set_probe(&probe);

        [[maybe_unused]] auto result = run(std::move(task));

        expect(eq(probe.awaited, true));
        expect(eq(probe.resumed, true));
        expect(eq(probe.done, true));
        expect(eq(probe.moved, false)); //rvalue used in-place
        expect(eq(static_cast<int>(probe.await_path), static_cast<int>(coroutine_probe::path::rvalue)));
    };

    "premature destruction"_test = [] mutable {
        coroutine_probe probe;

        echo(0).set_probe(&probe);

        expect(eq(probe.awaited, false));
        expect(eq(probe.done, false));
        expect(eq(probe.destroyed, true));
    };

    "premature destruction throws without probe"_test = [] mutable {
        bool threw = false;
        try {
            auto unawaited_task = echo(0);
        } catch (const std::logic_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };

    "move assignment sets moved"_test = [] mutable {
        coroutine_probe probe1;
        auto task1 = echo(5);
        task1.set_probe(&probe1);

        coroutine_probe probe2;
        auto task2 = echo(10);
        task2.set_probe(&probe2);

        task2       = std::move(task1); // move assignment
        auto result = run(task2);

        expect(eq(result, 5));
        expect(eq(probe1.moved, true));
        expect(eq(probe1.awaited, true));
        expect(eq(probe1.destroyed, false));
        expect(eq(probe2.moved, false));
        expect(eq(probe2.awaited, false));
        expect(eq(probe2.destroyed, true));
    };

    "self assignment is safe"_test = [] mutable {
        coroutine_probe probe;
        auto task = echo(42);
        task.set_probe(&probe);
        task = std::move(task);         // NOLINT(clang-diagnostic-self-move): testing safety of self-assignment
        expect(eq(probe.moved, false)); //self-assignment doesn't actually move
        expect(eq(run(task), 42));
    };

    "task factory"_test = [] mutable {
        coroutine_probe factory_probe;
        coroutine_probe task_probe;

        test_task<int> task;

        { //make sure factory is destroyed before we run the result task
            auto factory = make_echo(42, &task_probe);
            factory.set_probe(&factory_probe);
            task = run(factory);

            expect(eq(factory_probe.done, true));
        }
        expect(eq(factory_probe.destroyed, true));

        expect(eq(task_probe.moved, true));
        expect(eq(task_probe.awaited, false));
        expect(eq(task_probe.destroyed, false));

        auto result = run(task);

        expect(eq(result, 42));
        expect(eq(task_probe.awaited, true));
        expect(eq(task_probe.done, true));
    };

    "double await throws"_test = [] mutable {
        auto task = echo(42);

        [[maybe_unused]] auto result1 = run(task);

        bool threw = false;
        try {
            [[maybe_unused]] auto result2 = run(task);
        } catch (const std::logic_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };

    "exception propagates"_test = [] mutable {
        auto task = []() -> test_task<int> {
            throw std::runtime_error("boom");
            co_return 0;
        }();

        bool threw = false;
        try {
            [[maybe_unused]] auto result = run(task);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };

    "stalled coroutine detected"_test = [] mutable {
        coroutine_probe probe;

        auto task = [&]() -> test_task<void> { co_await std::suspend_always{}; }();

        task.set_probe(&probe);

        bool threw = false;
        try {
            run(task);
        } catch (const std::system_error& e) {
            if (e.code() == std::errc::resource_unavailable_try_again) {
                threw = true;
            }
        }
        expect(eq(threw, true));
        expect(eq(probe.awaited, true));
    };

    "nested coroutine await"_test = [] mutable {
        coroutine_probe probeA;
        coroutine_probe probeB;
        coroutine_probe probeC;
        coroutine_probe probeD;
        coroutine_probe probeE;

        // Lvalue task
        auto taskA = echo(42);
        taskA.set_probe(&probeA);

        // Temporary moved into taskB after probe is set
        auto taskB = echo(7).set_probe(&probeB);

        // Factory for rvalue task
        auto make_taskC = []() -> test_task<int> { co_return co_await echo(1); };

        auto taskE = [&]() -> test_task<int> {
            int quotient; //42 / 7 == 6
            co_await (
                [&]() -> test_task<void> {
                    quotient = (co_await taskA) / (co_await taskB);
                    co_return;
                }()
                             .set_probe(&probeD)
            );

            auto difference = quotient - co_await make_taskC().set_probe(&probeC); //6 - 1 == 5
            co_return difference;                                                  //5
        }();
        taskE.set_probe(&probeE);

        int result = run(taskE);
        expect(eq(result, 5));

        // Assertions for taskA (unmoved lvalue)
        expect(eq(probeA.awaited, true));
        expect(eq(probeA.suspended, false));
        expect(eq(probeA.resumed, true));
        expect(eq(probeA.moved, false));
        expect(eq(probeA.done, true));
        expect(eq(probeA.destroyed, false));
        expect(eq(static_cast<int>(probeA.await_path), static_cast<int>(coroutine_probe::path::lvalue)));

        // Assertions for taskB (moved lvalue)
        expect(eq(probeB.awaited, true));
        expect(eq(probeB.suspended, false));
        expect(eq(probeB.resumed, true));
        expect(eq(probeB.moved, true));
        expect(eq(probeB.done, true));
        expect(eq(probeB.destroyed, false));
        expect(eq(static_cast<int>(probeB.await_path), static_cast<int>(coroutine_probe::path::lvalue)));

        // Assertions for taskC (rvalue)
        expect(eq(probeC.awaited, true));
        expect(eq(probeC.suspended, true));
        expect(eq(probeC.resumed, true));
        expect(eq(probeC.moved, false)); // rvalue temporary is never moved after probe is attached
        expect(eq(probeC.done, true));
        expect(eq(probeC.destroyed, true));
        expect(eq(static_cast<int>(probeC.await_path), static_cast<int>(coroutine_probe::path::rvalue)));

        // Assertions for taskD (unmaterialized rvalue)
        expect(eq(probeD.awaited, true));
        expect(eq(probeD.suspended, true));
        expect(eq(probeD.resumed, true));
        expect(eq(probeD.moved, false));
        expect(eq(probeD.done, true));
        expect(eq(probeD.destroyed, true));
        expect(eq(static_cast<int>(probeD.await_path), static_cast<int>(coroutine_probe::path::rvalue)));

        // Assertions for taskE (lvalue)
        expect(eq(probeE.awaited, true));
        expect(eq(probeE.suspended, true));
        expect(eq(probeE.resumed, true));
        expect(eq(probeE.moved, false)); // rvalue returned by lambda used in-place
        expect(eq(probeE.done, true));
        expect(eq(probeE.destroyed, false));
        expect(eq(static_cast<int>(probeE.await_path), static_cast<int>(coroutine_probe::path::lvalue)));
    };

    // ============================================================
    // as_task adapter
    // ============================================================

    "as_task forwards value (ready path)"_test = [] mutable {
        // Bespoke trivial awaiter for bootstrapping tests. as_task tests can't depend on the test dummies namespace since their tests depend on as_task.
        struct echo_ready_awaiter {
            int value{};
            [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }
            constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
            [[nodiscard]] constexpr int await_resume() { return value; }
        };
        
        constexpr int expected = 55;
        auto result = run(as_task<int>(echo_ready_awaiter{expected}));
        expect(eq(result, expected));
    };

    "as_task preserves suspension semantics"_test = [] mutable {
        // Bespoke trivial awaiter for bootstrapping tests. as_task tests can't depend on the test dummies namespace since their tests depend on as_task.
        struct echo_immediate_awaiter {
            int value{};
            [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }
            [[nodiscard]] auto await_suspend(std::coroutine_handle<test_promise<int>> caller) noexcept
            {
                if (typename test_promise<int>::probe_ptr probe{caller.promise().probe}; probe)
                    probe->suspended = true;
                return caller; // symmetric transfer → resume caller right away
            }
            [[nodiscard]] constexpr int await_resume() { return value; }
        };

        constexpr int expected = 10;

        coroutine_probe probe;

        auto task = as_task<int>(echo_immediate_awaiter{expected});
        task.set_probe(&probe);

        auto result = run(task);

        expect(eq(result, expected));
        expect(eq(probe.suspended, true));
        expect(eq(probe.resumed, true));
    };

    "as_task propagates exception from await_resume"_test = [] mutable {
        struct throwing_awaiter {
            [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }
            constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
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
    
    "as_task supports move-only return types (unique_ptr)"_test = [] mutable {
        // Bespoke trivial awaiter for bootstrapping tests. as_task tests can't depend on the test dummies namespace since their tests depend on as_task.
        struct echo_ready_awaiter {
            std::unique_ptr<int> value{};
            [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }
            constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
            [[nodiscard]] constexpr std::unique_ptr<int> await_resume() { return std::move(value); }
        };

        int expected = 42;
        
        // Create an awaiter that holds a move-only type
        auto awaiter = echo_ready_awaiter{std::make_unique<int>(expected)};
    
        // Wrap it. This requires perfect forwarding to work inside 'as_task'
        auto task = as_task<std::unique_ptr<int>>(std::move(awaiter));
    
        // Run it. This requires the promise_type to correctly move the value out.
        std::unique_ptr<int> result = run(std::move(task));
    
        expect(eq(static_cast<bool>(result), true));
        if (result) {
            expect(eq(*result, expected));
        }
    };
};

suite coroutine_dummy_tests = [] mutable {
    // ============================================================
    // trivial_awaiter_base
    // ============================================================

    "trivial_awaiter_base returns stored value"_test = [] mutable {
        int expected = 42;
        dummies::trivial_awaiter_base<int> base{expected};
        expect(eq(base.await_resume(), expected));
    };

    "trivial_awaiter_base<void> is no-op"_test = [] mutable {
        dummies::trivial_awaiter_base<void> base;

        bool threw = false;
        try {
            base.await_resume();
        } catch (...) {
            threw = true;
        }
        expect(eq(threw, false));
        expect(eq(std::is_void_v<decltype(base.await_resume())>, true));
    };

    // ============================================================
    // ready_awaiter
    // ============================================================

    "ready_awaiter await_suspend throws (contract enforcement)"_test = [] mutable {
        dummies::ready_awaiter<int> awaiter{};

        bool threw = false;
        try {
            awaiter.await_suspend(std::noop_coroutine());
        } catch (const std::logic_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };

    "ready_awaiter returns value without suspension"_test = [] mutable {
        int expected = 42;

        coroutine_probe probe;

        auto task = as_task<int>(dummies::ready_awaiter{expected});
        task.set_probe(&probe);

        auto result = run(task);

        expect(eq(result, expected));
        expect(eq(probe.suspended, false));
    };

    "ready_awaiter<void> completes without suspension"_test = [] mutable {
        coroutine_probe probe;

        auto task = as_task<void>(dummies::ready_awaiter<void>{});
        task.set_probe(&probe);

        run(task);

        expect(eq(probe.suspended, false));
    };

    // ============================================================
    // immediate_awaiter
    // ============================================================

    "immediate_awaiter suspends and resumes"_test = [] mutable {
        int expected = 7;

        coroutine_probe probe;

        auto task = as_task<int>(dummies::immediate_awaiter{expected});

        task.set_probe(&probe);

        auto result = run(task);

        expect(eq(result, expected));
        expect(eq(probe.suspended, true));
        expect(eq(probe.done, true));
    };

    "immediate_awaiter<void> suspends and resumes"_test = [] mutable {
        coroutine_probe probe;

        auto task = as_task<void>(dummies::immediate_awaiter<void>{});

        task.set_probe(&probe);

        run(task);

        expect(eq(probe.suspended, true));
        expect(eq(probe.done, true));
    };

    // ============================================================
    // ADL awaitable
    // ============================================================

    "adl awaitable resolves via operator co_await"_test = [] mutable {
        int expected = 42;

        coroutine_probe probe;

        auto task = as_task<int>(dummies::adl::awaitable_by_adl<int>{expected});

        task.set_probe(&probe);

        auto result = run(task);

        expect(eq(result, expected));
        expect(eq(probe.suspended, false)); // uses ready_awaiter
    };
};

int main() {}
