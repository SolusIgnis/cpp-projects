// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:awaitables

import net.telnet;
import net.telnet.test_support;
import ut;
import std;

using namespace ut;
using namespace net::telnet::awaitables;
using namespace net::telnet::test_support::coroutine_harness;

suite net_telnet_awaitables_unit_tests = [] mutable {

    // ============================================================
    // Basic construction
    // ============================================================

    "tagged_awaitable default construction compiles"_test = [] mutable {
        tagged_awaitable<tags::option_enablement_tag, void> a{};
        expect(eq(true, true)); // existence test
    };

    "tagged_awaitable constructs from awaitable"_test = [] mutable {
        auto coro = []() -> test_task<void> { co_return; };

        tagged_awaitable<tags::option_enablement_tag, void, test_task<void>> a{coro()};
        expect(eq(true, true));
    };

    // ============================================================
    // Await semantics
    // ============================================================

    "tagged_awaitable forwards co_await result"_test = [] mutable {
        auto coro = []() -> test_task<int> {
            co_return 42;
        };

        tagged_awaitable<tags::option_enablement_tag, int, test_task<int>> a{coro()};

        auto test = [&]() -> test_task<int> {
            int v = co_await a;
            co_return v;
        };

        auto result = run(test());
        expect(eq(result, 42));
    };

    "tagged_awaitable supports rvalue co_await"_test = [] mutable {
        auto coro = []() -> test_task<int> {
            co_return 55;
        };

        auto test = [&]() -> test_task<int> {
            tagged_awaitable<tags::option_enablement_tag, int, test_task<int>> a{coro()};
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

        auto coro = [&]() -> test_task<int> {
            co_return 10;
        };

        auto wrapped = tagged_awaitable<tags::option_enablement_tag, int, test_task<int>>{coro()};
        wrapped.set_probe(&probe);

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
        auto sub = []() -> test_task<int> {
            co_return 7;
        };

        auto main = [&]() -> test_task<int> {
            tagged_awaitable<tags::option_enablement_tag, int, test_task<int>> a{sub()};
            int v = co_await a;
            co_return v * 3;
        };

        auto result = run(main());
        expect(eq(result, 21));
    };

    // ============================================================
    // Type aliases
    // ============================================================

    "option_enablement_awaitable is correctly typed"_test = [] mutable {
        bool same =
            std::is_same_v<
                option_enablement_awaitable,
                tagged_awaitable<tags::option_enablement_tag, void>
            >;

        expect(eq(same, true));
    };

    "option_disablement_awaitable is correctly typed"_test = [] mutable {
        bool same =
            std::is_same_v<
                option_disablement_awaitable,
                tagged_awaitable<tags::option_disablement_tag, void>
            >;

        expect(eq(same, true));
    };

    "subnegotiation_awaitable return type matches specification"_test = [] mutable {
        using expected =
            tagged_awaitable<
                tags::subnegotiation_tag,
                std::tuple<option, std::vector<byte_t>>
            >;

        bool same = std::is_same_v<subnegotiation_awaitable, expected>;
        expect(eq(same, true));
    };

    // ============================================================
    // Conversion to underlying awaitable
    // ============================================================

    "tagged_awaitable converts to underlying awaitable"_test = [] mutable {
        auto coro = []() -> test_task<int> {
            co_return 99;
        };

        tagged_awaitable<tags::option_enablement_tag, int, test_task<int>> a{coro()};

        test_task<int>& underlying = a;

        auto result = run(underlying);
        expect(eq(result, 99));
    };

};

int main() {}
