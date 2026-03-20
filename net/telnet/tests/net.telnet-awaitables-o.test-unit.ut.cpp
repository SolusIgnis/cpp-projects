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

tagged_awaitable<test_tag, int, test_task<int>> echo(int value)
{
    co_return value;
}

// Trivial awaiter that suspends once and immediately resumes via symmetric transfer
struct immediate_suspend_resume {
    int value;
    
    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) noexcept
    {
        return caller; // symmetric transfer → resume caller right away
    }

    constexpr int await_resume() const noexcept { return value; }
};

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
    // Type Safety and Tag Isolation
    // ============================================================

    "tagged_awaitables with different tags are distinct types"_test = [] mutable {
        struct foo_tag {};
        struct bar_tag {};
        
        using foo_t = tagged_awaitable<foo_tag, void, test_task<void>>;
        using bar_t = tagged_awaitable<bar_tag, void, test_task<void>>;

        // Verify they are not the same type
        expect(eq(std::same_as<foo_t, bar_t>, false));
        
        // Verify they are not cross-assignable or cross-constructible
        expect(eq(std::convertible_to<foo_t, bar_t>, false));
        expect(eq(std::convertible_to<bar_t, foo_t>, false));
        expect(eq(std::is_constructible_v<foo_t, bar_t>, false));
        expect(eq(std::is_constructible_v<bar_t, foo_t>, false));
    };

    // ============================================================
    // Await semantics
    // ============================================================

    "tagged_awaitable supports lvalue co_await"_test = [] mutable {
        coroutine_probe probe; 
        int expected = 42;

        auto coro = echo(expected);
        coro.get().set_probe(&probe);

        auto test = [&]() -> test_task<int> {
            int v = co_await coro;
            co_return v;
        };

        auto result = run(test());
        expect(eq(result, expected));
        expect(eq(static_cast<int>(probe.await_path), static_cast<int>(coroutine_probe::path::lvalue)));
    };

    "tagged_awaitable supports rvalue co_await"_test = [] mutable {
        coroutine_probe probe; 
        int expected = 42;

        auto coro = echo(expected);
        coro.get().set_probe(&probe);

        auto test = [&]() -> test_task<int> {
            int v = co_await std::move(coro);
            co_return v;
        };

        auto result = run(test());
        expect(eq(result, expected));
        expect(eq(static_cast<int>(probe.await_path), static_cast<int>(coroutine_probe::path::rvalue)));
    };
    
    "tagged_awaitable supports co_await of wrapped awaiter"_test = [] mutable {
        int expected = 42;
        
        tagged_awaitable<test_tag, void, immediate_suspend_resume> awaiter = immediate_suspend_resume{expected};
        
        auto coro = [&]() -> tagged_awaitable<test_tag, int, test_task<int>> { co_return co_await awaiter; };
        
        auto result = run(coro());
        
        expect(eq(result, expected));
    };

    // ============================================================
    // Coroutine probe lifecycle tracking
    // ============================================================

    "tagged_awaitable propagates coroutine lifecycle"_test = [] mutable {
        int expected = 42;

        coroutine_probe probe;

        auto wrapped = echo(expected);
        wrapped.get().set_probe(&probe);

        auto test = [&]() -> test_task<int> {
            co_return co_await wrapped;
        };

        auto result = run(test());

        expect(eq(result, expected));
        expect(eq(probe.awaited, true));
        expect(eq(probe.done, true));
    };

    // ============================================================
    // Nested awaitable composition
    // ============================================================

    "tagged_awaitable composes inside other coroutines"_test = [] mutable {
        int base     = 7;
        int mult     = 3;
        int expected = base * mult;

        auto sub = [&]() -> tagged_awaitable<test_tag, int, test_task<int>> {
            co_return base;
        };

        auto main = [&]() -> tagged_awaitable<test_tag, int, test_task<int>> {
            co_return (co_await sub()) * mult;
        };

        auto result = run(main());
        expect(eq(result, expected));
    };

    // ============================================================
    // Conversion to underlying awaitable
    // ============================================================

    "tagged_awaitable converts to underlying awaitable"_test = [] mutable {
        int expected = 42;
        auto wrapped{echo(expected)};

        auto run_underlying = [](test_task<int> task) { return run(task); };

        auto result = run_underlying(std::move(wrapped));
        expect(eq(result, expected));
    };
    
    // ============================================================
    // Coroutine Traits promise type
    // ============================================================

    "tagged_awaitable preserves underlying awaitable's promise type"_test = [] mutable {
        static_assert(std::same_as<test_task<int>::promise_type, std::coroutine_traits<tagged_awaitable<test_tag, int, test_task<int>>>::promise_type>);
        expect(eq(std::same_as<test_task<int>::promise_type, std::coroutine_traits<tagged_awaitable<test_tag, int, test_task<int>>>::promise_type>, true));
        expect(eq(std::same_as<test_task<void>::promise_type, std::coroutine_traits<tagged_awaitable<test_tag, void, test_task<void>>>::promise_type>, true));
    };
    
    "tagged_awaitable usable as coroutine return type"_test = [] mutable {
        auto tagged_echo_coro = [](int value) -> tagged_awaitable<test_tag, int, test_task<int>> { co_return value; };
        
        int expected = 42;
        
        auto result = run(tagged_echo_coro(expected));
        
        expect(eq(result, expected));
    };
    
    // ============================================================
    // Exception Propagation
    // ============================================================

    "tagged_awaitable propagates exceptions"_test = [] mutable {
        auto wrapped = [] (int value) -> tagged_awaitable<test_tag, int, test_task<int>> { throw std::runtime_error("boom"); co_return value; };

        int expected = 0;
        int result   = expected;
        int herring  = 42;
        
        bool threw = false;

        try {
            result = run(wrapped(herring));
        } catch (std::runtime_error&) {
            threw = true;
        }
        
        expect(eq(threw, true));
        expect(eq(result, expected));
    };
};

int main() {}
