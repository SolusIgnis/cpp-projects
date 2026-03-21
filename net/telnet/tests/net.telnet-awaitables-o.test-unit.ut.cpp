// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:awaitables

#include <asio.hpp>

import net.telnet;
import net.telnet.test_support;
import ut;
import std;

using namespace ut;
using namespace net::telnet::awaitables;
using namespace net::telnet::test_support::coroutine_harness;
using namespace std::literals;

struct test_tag {};

tagged_awaitable<test_tag, test_task<int>> echo(int value)
{
    co_return value;
}

// Trivial awaiter that suspends once and immediately resumes via symmetric transfer
template<typename T>
struct immediate_suspend_resume {
    T value{};
    
    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) const noexcept
    {
        return caller; // symmetric transfer → resume caller right away
    }

    constexpr auto await_resume() const noexcept { return value; }
};

namespace test_awaiting_adl {
    // Dummy type made awaitable by free operator co_await
    template<typename T>
    struct awaitable_by_adl {
        T value{};
    };
    
    template<typename T>
    auto operator co_await(awaitable_by_adl<T> dummy)
    {
        return immediate_suspend_resume<T>{dummy.value};
    }
} //namespace test_awaiting_adl

suite net_telnet_awaitables_unit_tests = [] mutable {
    // ============================================================
    // Basic construction
    // ============================================================

    "tagged_awaitable default construction compiles"_test = [] mutable {
        expect(eq(std::default_initializable<option_enablement_awaitable>, true));
        expect(eq(std::default_initializable<option_disablement_awaitable>, true));
        expect(eq(std::default_initializable<subnegotiation_awaitable>, true));
        expect(eq(std::is_default_constructible_v<tagged_awaitable<test_tag, test_task<int>>>, true));
    };

    "tagged_awaitable constructs from awaitable"_test = [] mutable {
        coroutine_probe probe;
        auto coro = []() -> test_task<void> { co_return; };

        tagged_awaitable<test_tag, test_task<void>> a{coro().set_probe(&probe)};

        expect(eq(probe.done, false));
    };

    // ============================================================
    // Type Safety and Tag Isolation
    // ============================================================

    "tagged_awaitables with identical tags and underlying value/awaitable types are the same type"_test = [] mutable {
        // Sanity check correlating with subsequent tests
        using foo_t = tagged_awaitable<test_tag, test_task<void>>;
        using bar_t = tagged_awaitable<test_tag, test_task<void>>;
        using baz_t = foo_t;

        // Verify they are the same type
        expect(eq(std::same_as<foo_t, bar_t>, true));
        expect(eq(std::same_as<foo_t, baz_t>, true));
        expect(eq(std::same_as<bar_t, baz_t>, true));
    };
    
    "tagged_awaitables with different tags are distinct types"_test = [] mutable {
        // Unique tags:
        struct foo_tag {};
        struct bar_tag {};

        // Applied to wrapped awaitables       
        using foo_t = tagged_awaitable<foo_tag, test_task<void>>;
        using bar_t = tagged_awaitable<bar_tag, test_task<void>>;

        // Verify they are not the same type
        expect(eq(std::same_as<foo_t, bar_t>, false));
        
        // Verify they are not cross-assignable or cross-constructible
        expect(eq(std::convertible_to<foo_t, bar_t>, false));
        expect(eq(std::convertible_to<bar_t, foo_t>, false));
        expect(eq(std::constructible_from<foo_t, bar_t>, false));
        expect(eq(std::constructible_from<bar_t, foo_t>, false));
    };
    
    "tagged_awaitables with different underlying awaitable types are distinct types"_test = [] mutable {
        using foo_t = tagged_awaitable<test_tag, immediate_suspend_resume<int>>;
        using bar_t = tagged_awaitable<test_tag, test_task<int>>;

        // Verify they are not the same type
        expect(eq(std::same_as<foo_t, bar_t>, false));
        
        // Verify they are not cross-assignable or cross-constructible
        expect(eq(std::convertible_to<foo_t, bar_t>, false));
        expect(eq(std::convertible_to<bar_t, foo_t>, false));
        expect(eq(std::constructible_from<foo_t, bar_t>, false));
        expect(eq(std::constructible_from<bar_t, foo_t>, false));
    };
    
    "option_enablement_awaitable and option_disablement_awaitable are exposed as distinct types"_test = [] mutable {
        // Verify they are not the same type
        expect(eq(std::same_as<option_enablement_awaitable, option_disablement_awaitable>, false));

        // Verify they are not cross-assignable or cross-constructible
        expect(eq(std::convertible_to<option_enablement_awaitable, option_disablement_awaitable>, false));
        expect(eq(std::convertible_to<option_disablement_awaitable, option_enablement_awaitable>, false));
        expect(eq(std::constructible_from<option_enablement_awaitable, option_disablement_awaitable>, false));
        expect(eq(std::constructible_from<option_disablement_awaitable, option_enablement_awaitable>, false));
    };
    
    // ============================================================
    // Zero runtime overhead
    // ============================================================

    "tagged_awaitable has zero size overhead"_test = [] mutable {
        auto tester = []<typename AwaitableT>() {
            using raw_t = AwaitableT;
            using tagged_t = tagged_awaitable<test_tag, AwaitableT>;
            using pathological_t = tagged_awaitable<std::array<int, 4>, AwaitableT>;
            
            // The wrapper should be exactly the size of the thing it wraps.
            expect(eq(sizeof(raw_t), sizeof(tagged_t)));
            expect(eq(sizeof(raw_t), sizeof(pathological_t))); 
            // It should also share the same alignment requirements.
            expect(eq(alignof(raw_t), alignof(tagged_t)));
            expect(eq(alignof(raw_t), alignof(pathological_t))); 
        };

        tester<test_task<void>>();
        tester<test_task<int>>();
        tester<test_task<std::array<int, 4>>>();
        
        tester<asio::awaitable<void>>();
        tester<asio::awaitable<int>>();
        tester<asio::awaitable<std::array<int, 4>>>();
    };

"check_alignment_glitch"_test = [] mutable {
    using task_t = test_task<void>;
    task_t tasks[2]; // Create an array
    auto delta = reinterpret_cast<char*>(&tasks[1]) - reinterpret_cast<char*>(&tasks[0]);
    
    // If delta is 16 but sizeof is 8, Clang's internal math is broken.
    expect(eq(static_cast<size_t>(delta), sizeof(task_t)));
    expect(eq(static_cast<size_t>(delta), static_cast<size_t>(16)));
    expect(eq(sizeof(task_t), static_cast<size_t>(16)));
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
    
    "tagged_awaitable supports co_await by free function (ADL)"_test = [] mutable {
        int expected = 42;
        
        using wrapper_t = tagged_awaitable<test_tag, test_awaiting_adl::awaitable_by_adl<int>>;
        // Note: This awaitable is trivially multi-shot because it doesn't require its own coroutine frame.
        wrapper_t wrapped = test_awaiting_adl::awaitable_by_adl{expected};
        
        expect(eq(run(as_task<int>(wrapped)), expected));
        expect(eq(run(as_task<int>(std::as_const(wrapped))), expected));
        expect(eq(run(as_task<int>(std::move(wrapped))), expected));
    };
    
    "tagged_awaitable supports co_await of wrapped awaiter"_test = [] mutable {
        int expected = 42;

        using wrapper_t = tagged_awaitable<test_tag, immediate_suspend_resume<int>>;
        // Note: This awaitable is trivially multi-shot because it doesn't require its own coroutine frame.
        wrapper_t wrapped = immediate_suspend_resume{expected};
        
        expect(eq(run(as_task<int>(wrapped)), expected));
        expect(eq(run(as_task<int>(std::as_const(wrapped))), expected));
        expect(eq(run(as_task<int>(std::move(wrapped))), expected));
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

        auto sub = [&]() -> tagged_awaitable<test_tag, test_task<int>> {
            co_return base;
        };

        auto main = [&]() -> tagged_awaitable<test_tag, test_task<int>> {
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
        static_assert(std::same_as<test_task<int>::promise_type, std::coroutine_traits<tagged_awaitable<test_tag, test_task<int>>>::promise_type>);
        expect(eq(std::same_as<test_task<int>::promise_type, std::coroutine_traits<tagged_awaitable<test_tag, test_task<int>>>::promise_type>, true));
        expect(eq(std::same_as<test_task<void>::promise_type, std::coroutine_traits<tagged_awaitable<test_tag, test_task<void>>>::promise_type>, true));
    };
    
    "tagged_awaitable usable as coroutine return type"_test = [] mutable {
        auto tagged_echo_coro = [](int value) -> tagged_awaitable<test_tag, test_task<int>> { co_return value; };
        
        int expected = 42;
        
        auto result = run(tagged_echo_coro(expected));
        
        expect(eq(result, expected));
    };
    
    // ============================================================
    // Exception Propagation
    // ============================================================

    "tagged_awaitable propagates exceptions"_test = [] mutable {
        auto wrapped = [] (int value) -> tagged_awaitable<test_tag, test_task<int>> { throw std::runtime_error("boom"); co_return value; };

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
