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
    
    "test_task default-constructs empty"_test = [] mutable {
       test_task<int> task1;
       test_task<void> task2;
       test_task<std::array<int,1024>> task3;
       
       expect(eq(static_cast<bool>(task1), false));
       expect(eq(static_cast<bool>(task2), false));
       expect(eq(static_cast<bool>(task3), false));
    };
    
    "run returns value"_test = [] mutable {
        constexpr int expected = 42;
        auto task = echo(expected);

        const auto result = run(task);

        expect(eq(result, expected));
    };

    "run returns void"_test = [] mutable {
        coroutine_probe probe;

        auto task = []() -> test_task<void> { co_return; }();

        task.set_probe(&probe);

        run(task);

        expect(eq(probe.awaited, true));
        expect(eq(probe.done, true));
    };

    "run throws on empty test_task"_test = [] mutable {
        test_task<void> task;
        
        bool threw = false;
        try {
            run(task);
        } catch (const std::logic_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };
    
    "operator co_await throws from empty test_task"_test = [] mutable {
        test_task<void> empty_task;
        
        bool threw = false;
        try {
            [[maybe_unused]] const auto awaiter = empty_task.operator co_await();
        } catch (const std::logic_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };

    "probe lifecycle"_test = [] mutable {
        coroutine_probe probe;

        {
            auto task = echo({});

            task.set_probe(&probe);

            [[maybe_unused]] const auto result = run(task);

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

        auto task = echo({});
        task.set_probe(&probe);

        [[maybe_unused]] const auto result = run(std::move(task));

        expect(eq(probe.awaited, true));
        expect(eq(probe.resumed, true));
        expect(eq(probe.done, true));
        expect(eq(probe.moved, false)); //rvalue used in-place
        expect(eq(static_cast<int>(probe.await_path), static_cast<int>(coroutine_probe::path::rvalue)));
    };

    "premature destruction"_test = [] mutable {
        coroutine_probe probe;

        echo({}).set_probe(&probe); //temporary object destroyed at the ;

        expect(eq(probe.awaited, false));
        expect(eq(probe.done, false));
        expect(eq(probe.destroyed, true));
    };

    "premature destruction throws without probe"_test = [] mutable {
        bool threw = false;
        try {
            auto unawaited_task = echo({});
        } catch (const std::logic_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };

    "swap exchanges tasks and preserves invariants"_test = [] mutable {
        constexpr int expected1 = 1; //A
        constexpr int expected2 = 2; //B
    
        coroutine_probe probe1{};
        coroutine_probe probe2{};
        {
            auto task1 = echo(expected1); //A
            auto task2 = echo(expected2); //B
        
            task1.set_probe(&probe1); //A
            task2.set_probe(&probe2); //B
        
            // Perform swap
            using std::swap;
            swap(task1, task2); //swap A and B
        
            // After swap, no lifecycle events should have happened yet
            expect(eq(probe1.destroyed, false));
            expect(eq(probe2.destroyed, false));
            expect(eq(probe1.awaited, false));
            expect(eq(probe2.awaited, false));
            expect(eq(probe1.moved, false));
            expect(eq(probe2.moved, false));
        
            const auto result1 = run(task1); // Run B
            
            // Probe behavior must follow the coroutine, not the wrapper
            expect(eq(probe1.awaited, false)); //A
            expect(eq(probe2.awaited, true));  //B
            
            const auto result2 = run(task2); // Run A

            // Probe behavior must follow the coroutine, not the wrapper
            expect(eq(probe1.awaited, true)); //A
        
            // Values must be swapped
            expect(eq(result1, expected2)); //B
            expect(eq(result2, expected1)); //A
        
            // Neither should be destroyed yet (still in scope)
            expect(eq(probe1.destroyed, false));
            expect(eq(probe2.destroyed, false));
        } // Destruction happens here
        expect(eq(probe1.destroyed, true));
        expect(eq(probe2.destroyed, true));
    };
    
    "swap is its own inverse operation (involution)"_test = [] mutable {
        constexpr int expected1 = 1; //A
        constexpr int expected2 = 2; //B
    
        coroutine_probe probe1{};
        coroutine_probe probe2{};
        
        auto task1 = echo(expected1); //A
        auto task2 = echo(expected2); //B
    
        task1.set_probe(&probe1); //A
        task2.set_probe(&probe2); //B
    
        // Perform double swap
        using std::swap;
        swap(task1, task2); //swap A and B
        swap(task1, task2); //swap B and A back
  
        const auto result1 = run(task1); // Run A
        
        // Probe behavior must follow the coroutine, not the wrapper
        expect(eq(probe1.awaited, true));  //A
        expect(eq(probe2.awaited, false)); //B
        
        const auto result2 = run(task2); // Run B

        // Probe behavior must follow the coroutine, not the wrapper
        expect(eq(probe2.awaited, true)); //B
    
        // Values must NOT be swapped
        expect(eq(result1, expected1)); //A
        expect(eq(result2, expected2)); //B
    };
    
    "self-swap is idempotent"_test = [] mutable {
        constexpr int expected = 42;
    
        coroutine_probe probe{};
    
        {
            auto task = echo(expected);
            task.set_probe(&probe);
    
            // Perform self-swap
            using std::swap;
            swap(task, task);
    
            // After swap, no lifecycle events should have happened yet
            expect(eq(probe.destroyed, false));
            expect(eq(probe.awaited, false));
            expect(eq(probe.moved, false));
    
            // Still behaves normally
            const auto result = run(task);
            expect(eq(result, expected));
    
            expect(eq(probe.awaited, true));
    
            // Should not be destroyed yet (still in scope)
            expect(eq(probe.destroyed, false));
        } // Destruction happens here
        expect(eq(probe.destroyed, true));
    };

    "move assignment sets moved and destroys assigned-to"_test = [] mutable {
        constexpr int expected  = 5;
        constexpr int discarded = 10;
        
        coroutine_probe probe1;
        coroutine_probe probe2;
        
        auto task2 = echo(discarded);
        task2.set_probe(&probe2);
        {
            auto task1 = echo(expected);
            task1.set_probe(&probe1);

            task2 = std::move(task1); // move assignment
            expect(eq(probe1.moved, true));
            expect(eq(probe2.destroyed, false));
        } //destruction of discarded task occurs here when task1 destructor runs
        const auto result = run(task2);

        expect(eq(result, expected));
        expect(eq(probe1.moved, true));
        expect(eq(probe1.awaited, true));
        expect(eq(probe1.destroyed, false));
        expect(eq(probe2.moved, false));
        expect(eq(probe2.awaited, false));
        expect(eq(probe2.destroyed, true));
    };

    "self assignment is safe"_test = [] mutable {
        constexpr int expected = 42;
        coroutine_probe probe;

        auto task = echo(expected);
        task.set_probe(&probe);
        task = std::move(task);         // NOLINT(clang-diagnostic-self-move): testing safety of self-assignment
        expect(eq(probe.moved, false)); //self-assignment doesn't actually move
        expect(eq(run(task), expected));
    };

    "task factory"_test = [] mutable {
        constexpr int expected = 42;

        coroutine_probe factory_probe;
        coroutine_probe task_probe;

        test_task<int> task;

        { //make sure factory is destroyed before we run the result task
            auto factory = make_echo(expected, &task_probe);
            factory.set_probe(&factory_probe);
            task = run(factory);

            expect(eq(factory_probe.done, true));
        }
        expect(eq(factory_probe.destroyed, true));

        expect(eq(task_probe.moved, true));
        expect(eq(task_probe.awaited, false));
        expect(eq(task_probe.destroyed, false));

        const auto result = run(task);

        expect(eq(result, expected));
        expect(eq(task_probe.awaited, true));
        expect(eq(task_probe.done, true));
    };

    "double await throws"_test = [] mutable {
        auto task = echo({});

        [[maybe_unused]] const auto result1 = run(task);

        bool threw = false;
        try {
            [[maybe_unused]] const auto result2 = run(task);
        } catch (const std::logic_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };
    
    "double await across swap throws"_test = [] mutable {
        auto task1 = echo({}); //A
        auto task2 = echo({}); //B

        [[maybe_unused]] const auto result1 = run(task1); //run A as task1
        
        using std::swap;
        swap(task1, task2); //swap A and B

        {
            bool threw = false;
            try {
                [[maybe_unused]] const auto result2 = run(task2); //run A as task2
            } catch (const std::logic_error&) {
                threw = true;
            }
            expect(eq(threw, true));
        }
        {
            bool threw = false;
            try {
                [[maybe_unused]] const auto result3 = run(task1); //run B as task1
            } catch (const std::logic_error&) {
                threw = true;
            }
            expect(eq(threw, false));
        }
    };
    
    "double await across move construction throws"_test = [] mutable {
        auto task1 = echo({});

        [[maybe_unused]] const auto result1 = run(task1);
        
        auto task2 = std::move(task1); //move construction

        expect(eq(static_cast<bool>(task1), false));

        bool threw = false;
        try {
            [[maybe_unused]] const auto result2 = run(task2);
        } catch (const std::logic_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };

    "double await across move assignment throws"_test = [] mutable {
        auto task1 = echo({});
        decltype(echo({})) task2;

        [[maybe_unused]] const auto result1 = run(task1);
        
        task2 = std::move(task1); //move assignment

        expect(eq(static_cast<bool>(task1), false));

        bool threw = false;
        try {
            [[maybe_unused]] const auto result2 = run(task2);
        } catch (const std::logic_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };

    "exception propagates"_test = [] mutable {
        auto task = []() -> test_task<int> {
            throw std::runtime_error("boom");
            co_return {};
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
        constexpr int dividend   = 42;
        constexpr int divisor    = 7;
        constexpr int subtrahend = 1;
        constexpr int expected   = (dividend / divisor) - subtrahend;
        
        coroutine_probe probeA;
        coroutine_probe probeB;
        coroutine_probe probeC;
        coroutine_probe probeD;
        coroutine_probe probeE;

        // Lvalue task
        auto taskA = echo(dividend);
        taskA.set_probe(&probeA);

        // Temporary moved into taskB after probe is set
        auto taskB = echo(divisor).set_probe(&probeB);

        // Factory for rvalue task
        auto make_taskC = []() -> test_task<int> { co_return co_await echo(subtrahend); };

        auto taskE = [&]() -> test_task<int> {
            int quotient; //42 / 7 == 6
            co_await (
                [&]() -> test_task<void> {
                    quotient = (co_await taskA) / (co_await taskB);
                    co_return;
                }()
                             .set_probe(&probeD)
            );

            const auto difference = quotient - co_await make_taskC().set_probe(&probeC); //6 - 1 == 5
            co_return difference;                                                  //5
        }();
        taskE.set_probe(&probeE);

        const int result = run(taskE);
        expect(eq(result, expected));

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
    
    "continuation chaining preserves strict resume order"_test = [] mutable {
        std::vector<int> trace;
        constexpr int first  = 1;
        constexpr int second = 2;
        constexpr int third  = 3;
        constexpr int fourth = 4;
        constexpr int fifth  = 5;
        std::vector<int> expected{first, second, third, fourth, fifth};
    
        auto leaf = [&]() -> test_task<void> {
            trace.push_back(third);
            co_return;
        };
    
        auto mid = [&]() -> test_task<void> {
            trace.push_back(second);
            co_await leaf();
            trace.push_back(fourth);
        };
    
        auto root = [&]() -> test_task<void> {
            trace.push_back(first);
            co_await mid();
            trace.push_back(fifth);
        };
    
        run(root());
    
        expect(eq(trace.size(), expected.size()));
        if (trace.size() == expected.size()) {
            for (std::size_t i = 0, size = trace.size(); i < size; ++i) {
              expect(eq(trace.at(i), expected.at(i)));
            }
        }
    };
};

suite as_task_adapter_tests = [] mutable {
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
        const auto result = run(as_task<int>(echo_ready_awaiter{expected}));
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

        const auto result = run(task);

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
    
    "as_task propagates exception from await_suspend"_test = [] mutable {
        struct throwing_suspend_awaiter {
            constexpr bool await_ready() const noexcept { return false; }
            [[noreturn]] void await_suspend(std::coroutine_handle<>) {
                throw std::runtime_error("boom");
            }
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
            std::unique_ptr<int> value{};
            [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }
            constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
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
        auto result = run(as_task<int>(echo(expected)));

        expect(eq(result, expected));
    };
    
    "as_task supports nested calls"_test = [] mutable {
        // Bespoke trivial awaiter for bootstrapping tests. as_task tests can't depend on the test dummies namespace since their tests depend on as_task.
        struct echo_ready_awaiter {
            int value{};
            [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }
            constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
            [[nodiscard]] constexpr int await_resume() { return value; }
        };
        int expected = 42;

        auto taskA = as_task<int>(echo_ready_awaiter{expected});
        auto taskB = as_task<int>(as_task<int>(std::move(taskA)));

        auto result = run(taskB);
        expect(eq(result, expected));
    };
};

template<typename AwaiterT, typename T>
concept const_lvalue_resumable = requires(T& result, AwaiterT& awaiter) { result = std::as_const(awaiter).await_resume(); };

suite dummy_awaitable_tests = [] mutable {
    // ============================================================
    // trivial_awaiter_base
    // ============================================================

    "trivial_awaiter_base returns stored value"_test = [] mutable {
        constexpr int expected = 42;
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

    "trivial_awaiter_base supports move-only return types (unique_ptr)"_test = [] mutable {
        constexpr int expected = 42;
        dummies::trivial_awaiter_base<std::unique_ptr<int>> base{std::make_unique<int>(expected)};

        const auto result = base.await_resume();
        
        expect(eq(static_cast<bool>(result), true));
        if (result) {
            expect(eq(*result, expected));
        }
    };
    
    "trivial_awaiter_base await_resume value category correctness"_test = [] mutable {
        bool lvalue       = requires(int& result, dummies::trivial_awaiter_base<int>& base) { result = base.await_resume(); };
        bool rvalue       = requires(int& result, dummies::trivial_awaiter_base<int>& base) { result = std::move(base).await_resume(); };
        bool const_lvalue = const_lvalue_resumable<dummies::trivial_awaiter_base<int>, int>;
        bool clvalue_move = const_lvalue_resumable<dummies::trivial_awaiter_base<std::unique_ptr<int>>, std::unique_ptr<int>>;
        
        expect(eq(const_lvalue, true));
        expect(eq(lvalue, true));
        expect(eq(rvalue, true));
        expect(eq(clvalue_move, false));
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

        const auto result = run(task);

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

        const auto result = run(task);

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
    
    "immediate_awaiter fallback path (non-test_task)"_test = [] mutable {
        dummies::immediate_awaiter<void> awaiter{};
    
        auto coro = std::noop_coroutine();
        const auto result = awaiter.await_suspend(coro);
    
        // This confirms immediate symmetric transfer back to the caller.
       expect(eq(result.address(), coro.address()));
    };

    // ============================================================
    // ADL awaitable
    // ============================================================

    "adl awaitable resolves via operator co_await"_test = [] mutable {
        int expected = 42;

        coroutine_probe probe;

        auto task = as_task<int>(dummies::adl::awaitable_by_adl<int>{expected});

        task.set_probe(&probe);

        const auto result = run(task);

        expect(eq(result, expected));
        expect(eq(probe.suspended, false)); // uses ready_awaiter
    };
};
#if 1
suite coroutine_harness_integration_tests = [] mutable {
    "mega coroutine harness: full integration"_test = [] mutable {
        constexpr int first   = 1;
        constexpr int second  = 2;
        constexpr int third   = 3;
        constexpr int fourth  = 4;
        constexpr int fifth   = 5;
        constexpr int sixth   = 6;
        constexpr int seventh = 7;
        
        std::vector<int> trace;
    
        coroutine_probe probeIntLvalue, probeIntRvalue;
        coroutine_probe probePtr, probeVoid, probeThrow, probeNested;
    
        // -------------------------------
        // Leaf tasks
        // -------------------------------
        
        // int via ready_awaiter (lvalue)
        auto leafIntL = [&]() -> test_task<int> {
            trace.push_back(second);
            co_return run(as_task<int>(dummies::ready_awaiter{42}));
        };
    
        // int via ready_awaiter (rvalue)
        auto leafIntR = [&]() -> test_task<int> {
            trace.push_back(third);
            co_return run(as_task<int>(dummies::ready_awaiter{58}));
        };
    
        // unique_ptr via immediate_awaiter
        auto leafPtr = [&]() -> test_task<std::unique_ptr<int>> {
            trace.push_back(fourth);
            auto awaiter = dummies::immediate_awaiter{std::make_unique<int>(99)};
            co_return run(as_task<std::unique_ptr<int>>(std::move(awaiter)));
        };
    
        // void via immediate_awaiter
        auto leafVoid = [&]() -> test_task<void> {
            trace.push_back(fifth);
            co_await as_task<void>(dummies::immediate_awaiter<void>{});
        };
    
        // throwing awaiter
        auto leafThrow = [&]() -> test_task<void> {
            trace.push_back(sixth);
            struct throwing_awaiter {
                constexpr bool await_ready() const noexcept { return true; }
                void await_suspend(std::coroutine_handle<>) const noexcept {}
                [[noreturn]] void await_resume() const { throw std::runtime_error("boom"); }
            };
            co_await as_task<void>(throwing_awaiter{});
        };
    
        // -------------------------------
        // Nested composition task
        // -------------------------------
        auto nested = [&]() -> test_task<int> {
            trace.push_back(first);
    
            int valL = co_await leafIntL().set_probe(&probeIntLvalue); // 42
            int valR = co_await std::move(leafIntR().set_probe(&probeIntRvalue)); // 58
            auto ptr  = co_await leafPtr().set_probe(&probePtr); // 99
            co_await leafVoid().set_probe(&probeVoid);          // void task
    
            // Exception propagation check
            bool threw = false;
            try {
                co_await leafThrow().set_probe(&probeThrow);
            } catch (const std::runtime_error&) {
                threw = true;
            }
            expect(eq(threw, true));
    
            trace.push_back(seventh);
            co_return valL + valR + *ptr; // 42 + 58 + 99 == 199
        }();
    
        nested.set_probe(&probeNested);
    
        // -------------------------------
        // Run mega-task
        // -------------------------------
        int result = run(nested);
    
        // -------------------------------
        // Verify result
        // -------------------------------
        expect(eq(result, 199));
    
        // -------------------------------
        // Verify trace order
        // -------------------------------
        std::vector<int> expectedTrace{first, second, third, fourth, fifth, sixth, seventh};
        expect(eq(trace.size(), expectedTrace.size()));
        for (std::size_t i = 0; i < trace.size(); ++i) {
            expect(eq(trace[i], expectedTrace[i]));
        }
    
        // -------------------------------
        // Verify probes for all leaf tasks
        // -------------------------------
    
        // leafIntL (lvalue)
        expect(eq(probeIntLvalue.awaited, true));
        expect(eq(probeIntLvalue.resumed, true));
        expect(eq(probeIntLvalue.done, true));
        expect(eq(probeIntLvalue.suspended, false));
        expect(eq(probeIntLvalue.moved, false));
        expect(eq(probeIntLvalue.destroyed, true));
    
        // leafIntR (rvalue)
        expect(eq(probeIntRvalue.awaited, true));
        expect(eq(probeIntRvalue.resumed, true));
        expect(eq(probeIntRvalue.done, true));
        expect(eq(probeIntRvalue.suspended, false));
        expect(eq(probeIntRvalue.moved, false));
        expect(eq(probeIntRvalue.destroyed, true));
    
        // leafPtr
        expect(eq(probePtr.awaited, true));
        expect(eq(probePtr.done, true));
        expect(eq(probePtr.resumed, true));
        expect(eq(probePtr.suspended, false));
        expect(eq(probePtr.moved, false));
        expect(eq(probePtr.destroyed, true));
    
        // leafVoid
        expect(eq(probeVoid.awaited, true));
        expect(eq(probeVoid.done, true));
        expect(eq(probeVoid.resumed, true));
        expect(eq(probeVoid.suspended, true));
        expect(eq(probeVoid.moved, false));
        expect(eq(probeVoid.destroyed, true));
    
        // leafThrow
        expect(eq(probeThrow.awaited, true));
        expect(eq(probeThrow.done, true));
        expect(eq(probeThrow.resumed, true));
        expect(eq(probeThrow.suspended, true));
        expect(eq(probeThrow.moved, false));
        expect(eq(probeThrow.destroyed, true));
    
        // nested
        expect(eq(probeNested.awaited, true));
        expect(eq(probeNested.done, true));
        expect(eq(probeNested.resumed, true));
        expect(eq(probeNested.suspended, true));
        expect(eq(probeNested.moved, false));
        expect(eq(probeNested.destroyed, false));
    };
};
#endif
int main() {}
