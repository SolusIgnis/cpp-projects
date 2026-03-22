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

struct test_tag;

tagged_awaitable<test_tag, test_task<int>> echo(int value)
{
    co_return value;
}

namespace dummies {
///@brief Base class for trivial awaiters handling storage and value return from resume.
template<typename T>
struct trivial_awaiter_base {
protected:
    // Only store value if T is not void
    std::conditional_t<std::is_void_v<T>, std::monostate, T> storage_{};

public:
    constexpr trivial_awaiter_base() = default;
    constexpr trivial_awaiter_base(T val) noexcept(std::is_nothrow_constructible_v<T>)
        : storage_(val) {}

    constexpr auto await_resume() const noexcept {
        if constexpr (std::is_void_v<T>) {
            return; // void optimization
        } else {
            return storage_;
        }
    }
};

///@brief Trivial awaiter that is always ready.
template<typename T>
struct ready_awaiter : trivial_awaiter_base<T> {
    using base = trivial_awaiter_base<T>;
    using base::base;

    constexpr bool await_ready() const noexcept { return true; }

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

    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) const noexcept {
        return caller; // symmetric transfer → resume caller right away
    }
};

template<typename T>
immediate_awaiter(T) -> immediate_awaiter<T>;

namespace adl {
    // Dummy type made awaitable by free operator co_await
    template<typename T>
    struct awaitable_by_adl {
        T value{};
    };
    
    template<typename T>
    auto operator co_await(awaitable_by_adl<T> dummy)
    {
        return ready_awaiter<T>{dummy.value};
    }
} //namespace adl
} //namespace dummies

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
        struct foo_tag;
        struct bar_tag;

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
        using foo_t = tagged_awaitable<test_tag, dummies::immediate_awaiter<int>>;
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
        auto tester = [] <typename AwaitableT> () {
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

        tester.operator()<test_task<void>>();
        tester.operator()<test_task<int>>();
        tester.operator()<test_task<std::array<int, 4>>>();

        tester.operator()<dummies::ready_awaiter<void>>();
        tester.operator()<dummies::ready_awaiter<int>>();
        tester.operator()<dummies::ready_awaiter<std::array<int, 4>>>();

        tester.operator()<dummies::immediate_awaiter<void>>();
        tester.operator()<dummies::immediate_awaiter<int>>();
        tester.operator()<dummies::immediate_awaiter<std::array<int, 4>>>();

        tester.operator()<dummies::adl::awaitable_by_adl<void>>();
        tester.operator()<dummies::adl::awaitable_by_adl<int>>();
        tester.operator()<dummies::adl::awaitable_by_adl<std::array<int, 4>>>();
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
        
        using wrapper_t = tagged_awaitable<test_tag, dummies::adl::awaitable_by_adl<int>>;
        // Note: This awaitable is trivially multi-shot because it doesn't require its own coroutine frame.
        wrapper_t wrapped = dummies::adl::awaitable_by_adl{expected};
        
        expect(eq(run(as_task<int>(wrapped)), expected));
        expect(eq(run(as_task<int>(std::as_const(wrapped))), expected));
        expect(eq(run(as_task<int>(std::move(wrapped))), expected));
    };
    
    "tagged_awaitable supports co_await of wrapped awaiter"_test = [] mutable {
        int expected = 42;

        using wrapper_t = tagged_awaitable<test_tag, dummies::immediate_awaiter<int>>;
        // Note: This awaitable is trivially multi-shot because it doesn't require its own coroutine frame.
        wrapper_t wrapped = dummies::immediate_awaiter{expected};
        
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
    
    "tagged_awaitable propagates destruction to underlying awaitable"_test = [] mutable {
        coroutine_probe probe;

        {
            //Wrap a task that hasn't started or finished
            auto wrapped = echo({});
            wrapped.get().set_probe(&probe);

            expect(eq(probe.done, false));
            expect(eq(probe.destroyed, false));
        } //`wrapped` goes out of scope here. 

        //Verify the destruction worked
        expect(eq(probe.done, false));
        expect(eq(probe.destroyed, true)); 
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
