// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet.test_support:coroutine_harness

import net.telnet.test_support;
import ut;
import std;

using namespace ut;
using namespace net::telnet::test_support::coroutine_harness;

test_task<int> echo(int value)
{
    co_return value;
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
        expect(eq(probe.moved, true));
        expect(eq(static_cast<int>(probe.await_path), static_cast<int>(coroutine_probe::path::rvalue)));
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

        // Lvalue tasks
        auto taskA = echo(42).set_probe(&probeA);
        auto taskB = echo(7).set_probe(&probeB);

        // Factory for rvalue task
        auto make_taskC = []() -> test_task<int> { co_return co_await echo(1); };

        auto taskE = [&]() -> test_task<int> {
            auto foo        = co_await taskA;  //42
            auto bar        = co_await taskB;  //7

            int quotient;
            auto taskD = [&]() -> test_task<void> { quotient = foo / bar; co_return; }();
            co_await taskD; //42 / 7 == 6
            
            auto difference = quotient - co_await make_taskC().set_probe(&probeC); //6 - 1 == 5
            co_return difference; //5
        }().set_probe(&probeE);

        int result = run(taskE);
        expect(eq(result, 5));

        // Assertions for taskA (lvalue)
        expect(eq(probeA.awaited, true));
        expect(eq(probeA.suspended, false));
        expect(eq(probeA.resumed, true));
        expect(eq(probeA.moved, false));
        expect(eq(probeA.done, true));
        expect(eq(probeA.destroyed, true));
        expect(eq(static_cast<int>(probeA.await_path), static_cast<int>(coroutine_probe::path::lvalue)));

        // Assertions for taskB (lvalue)
        expect(eq(probeB.awaited, true));
        expect(eq(probeB.suspended, false));
        expect(eq(probeB.resumed, true));
        expect(eq(probeB.moved, false));
        expect(eq(probeB.done, true));
        expect(eq(probeB.destroyed, true));
        expect(eq(static_cast<int>(probeB.await_path), static_cast<int>(coroutine_probe::path::lvalue)));

        // Assertions for taskC (rvalue)
        expect(eq(probeC.awaited, true));
        expect(eq(probeC.suspended, true));
        expect(eq(probeC.resumed, true));
        expect(eq(probeC.moved, true)); // rvalue was moved
        expect(eq(probeC.done, true));
        expect(eq(probeC.destroyed, true));
        expect(eq(static_cast<int>(probeC.await_path), static_cast<int>(coroutine_probe::path::rvalue)));

        // Assertions for taskD (lvalue)
        expect(eq(probeD.awaited, true));
        expect(eq(probeD.suspended, false));
        expect(eq(probeD.resumed, true));
        expect(eq(probeD.moved, false));
        expect(eq(probeD.done, true));
        expect(eq(probeD.destroyed, true));
        expect(eq(static_cast<int>(probeD.await_path), static_cast<int>(coroutine_probe::path::lvalue)));

        // Assertions for taskE (rvalue)
        expect(eq(probeE.awaited, true));
        expect(eq(probeE.suspended, true));
        expect(eq(probeE.resumed, true));
        expect(eq(probeE.moved, true)); // rvalue returned by lambda
        expect(eq(probeE.done, true));
        expect(eq(probeE.destroyed, true));
        expect(eq(static_cast<int>(probeE.await_path), static_cast<int>(coroutine_probe::path::rvalue)));
    };
};
