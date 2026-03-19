// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:awaitables

import net.telnet;
import net.telnet.test_support;
import ut;
import std;

using namespace ut;
using namespace net::telnet::awaitables;
using namespace net::telnet::test_support::coroutine_harness;
using namespace std::literals;

struct test_tag {};

suite net_telnet_awaitables_unit_tests = [] mutable {
    // ============================================================
    // Basic construction
    // ============================================================

    "tagged_awaitable default construction compiles"_test = [] mutable {
        expect(eq(std::default_initializable<option_enablement_awaitable>, true));
        expect(eq(std::default_initializable<option_disablement_awaitable>, true));
        expect(eq(std::default_initializable<subnegotiation_awaitable>, true));
        expect(eq(std::is_default_constructible_v<tagged_awaitable<test_tag, int, test_task<int>>>, true));
    };

    "tagged_awaitable constructs from awaitable"_test = [] mutable {
        coroutine_probe probe;
        auto coro = []() -> test_task<void> { co_return; };

        tagged_awaitable<test_tag, void, test_task<void>> a{coro().set_probe(&probe)};

        expect(eq(probe.done, false));
    };

    // ============================================================
    // Await semantics
    // ============================================================

    "tagged_awaitable forwards co_await result"_test = [] mutable {
        auto coro = []() -> test_task<int> { co_return 42; };

        tagged_awaitable<test_tag, int, test_task<int>> a{coro()};

        auto test = [&]() -> test_task<int> {
            int v = co_await a;
            co_return v;
        };

        auto result = run(test());
        expect(eq(result, 42));
    };

    "tagged_awaitable supports rvalue co_await"_test = [] mutable {
        auto coro = []() -> test_task<int> { co_return 55; };

        auto test = [&]() -> test_task<int> {
            tagged_awaitable<test_tag, int, test_task<int>> a{coro()};
            int v = co_await std::move(a);
            co_return v;
        };

        auto result = run(test());
        expect(eq(result, 55));
    };

    // ============================================================
    // Coroutine probe lifecycle tracking
    // ============================================================

    "tagged_awaitable propagates coroutine lifecycle"_test = [] mutable {
        coroutine_probe probe;

        auto coro = [&]() -> test_task<int> { co_return 10; };

        auto wrapped = tagged_awaitable<test_tag, int, test_task<int>>{coro().set_probe(&probe)};

        auto test = [&]() -> test_task<int> {
            int value = co_await wrapped;
            co_return value + 5;
        };

        auto result = run(test());

        expect(eq(result, 15));
        expect(eq(probe.awaited, true));
        expect(eq(probe.done, true));
    };

    // ============================================================
    // Nested awaitable composition
    // ============================================================

    "tagged_awaitable composes inside other coroutines"_test = [] mutable {
        auto sub = []() -> test_task<int> { co_return 7; };

        auto main = [&]() -> test_task<int> {
            tagged_awaitable<test_tag, int, test_task<int>> a{sub()};
            int v = co_await a;
            co_return v * 3;
        };

        auto result = run(main());
        expect(eq(result, 21));
    };

    // ============================================================
    // Conversion to underlying awaitable
    // ============================================================

    "tagged_awaitable converts to underlying awaitable"_test = [] mutable {
        auto coro = []() -> test_task<int> { co_return 99; };

        tagged_awaitable<test_tag, int, test_task<int>> wrapped{coro()};

        auto run_underlying = [](test_task<int> task) { return run(task); };

        auto result = run_underlying(std::move(wrapped));
        expect(eq(result, 99));
    };
};

int main() {}
