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
        expect(eq(probe.await_path, coroutine_probe::path::none));
    };

    "probe lifecycle"_test = [] mutable {
        coroutine_probe probe;

        {
            auto task = echo(42);
        
            task.set_probe(&probe);

            [[maybe_unused]] auto result = run(task);

            expect(eq(probe.awaited, true));
            expect(eq(probe.resumed, true));
            expect(eq(probe.done, true));
            expect(eq(probe.destroyed, false));
            expect(eq(probe.moved, false));
            expect(eq(probe.await_path, coroutine_probe::path::lvalue));
        }

        expect(eq(probe.destroyed, true));
    };

    "rvalue await path"_test = [] mutable {
        coroutine_probe probe;

        auto task = echo(42);

        task.set_probe(&probe);

        [[maybe_unused]] auto result = run(std::move(task));

        expect(eq(probe.awaited, true));
        expect(eq(probe.moved, true));
        expect(eq(probe.await_path, coroutine_probe::path::rvalue));
    };

    "double await throws"_test = [] mutable {
        auto task = echo(42);

        [[maybe_unused]] auto result = run(task);

        expect(throws<std::logic_error>([&] {
            run(task);
        }));
    };

    "exception propagates"_test = [] mutable {
        auto task = []() -> test_task<int> {
            throw std::runtime_error("boom");
            co_return 0;
        }();

        expect(throws<std::runtime_error>([&] {
            run(task);
        }));
    };

    "stalled coroutine detected"_test = [] mutable {
        coroutine_probe probe;

        auto task = [&]() -> test_task<void> {
            co_await std::suspend_always{};
        }();

        task.set_probe(&probe);

        expect(throws<std::system_error>([&] {
            run(task);
        }));

        expect(probe.awaited);
    };

    "nested coroutine await"_test = [] mutable {
        auto child = []() -> test_task<int> {
            co_return 5;
        };

        auto parent = [&]() -> test_task<int> {
            co_return co_await child();
        };

        expect(5_i == run(parent()));
    };
};
