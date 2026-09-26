// SPDX-License-Identifier: Apache-2.0
// Unit tests for framework.coroutines.tagged_awaitable

import framework.coroutines.tagged_awaitable;

import tools.test.coroutine_harness;
import boost.ut;
import std;

using namespace boost::ext::ut;
using framework::coroutines::tagged_awaitable;
using namespace tools::test::coroutine_harness;
using namespace std::literals;

namespace {
    struct test_tag;

    tagged_awaitable<test_tag, test_task<std::int32_t>> echo(std::int32_t value)
    {
        co_return value;
    }
} //namespace

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    // ============================================================
    // Basic construction
    // ============================================================

    "tagged_awaitable default constructible"_test = [] mutable {
        expect(that % std::is_default_constructible_v<tagged_awaitable<test_tag, test_task<std::int32_t>>>);
    };

    "tagged_awaitable constructs from awaitable"_test = [] mutable {
        coroutine_probe probe;
        const auto coro = [] -> test_task<void> { co_return; };

        const tagged_awaitable<test_tag, test_task<void>> tagged{coro().set_probe(&probe)};

        expect(that % !probe.done);
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
        expect(eq(that % std::same_as<foo_t, bar_t>);
        expect(eq(that % std::same_as<foo_t, baz_t>);
        expect(eq(that % std::same_as<bar_t, baz_t>);
    };

    "tagged_awaitables with different tags are distinct types"_test = [] mutable {
        // Unique tags:
        struct foo_tag;
        struct bar_tag;

        // Applied to wrapped awaitables
        using foo_t = tagged_awaitable<foo_tag, test_task<void>>;
        using bar_t = tagged_awaitable<bar_tag, test_task<void>>;

        // Verify they are not the same type
        expect(that % !std::same_as<foo_t, bar_t>);

        // Verify they are not cross-assignable or cross-constructible
        expect(that % !std::convertible_to<foo_t, bar_t>);
        expect(that % !std::convertible_to<bar_t, foo_t>);
        expect(that % !std::constructible_from<foo_t, bar_t>);
        expect(that % !std::constructible_from<bar_t, foo_t>);
    };

    "tagged_awaitables with different underlying awaitable types are distinct types"_test = [] mutable {
        using foo_t = tagged_awaitable<test_tag, dummies::immediate_awaiter<std::int32_t>>;
        using bar_t = tagged_awaitable<test_tag, test_task<std::int32_t>>;

        // Verify they are not the same type
        expect(that % !std::same_as<foo_t, bar_t>);

        // Verify they are not cross-assignable or cross-constructible
        expect(that % !std::convertible_to<foo_t, bar_t>);
        expect(that % !std::convertible_to<bar_t, foo_t>);
        expect(that % !std::constructible_from<foo_t, bar_t>);
        expect(that % !std::constructible_from<bar_t, foo_t>);
    };

    // ============================================================
    // Zero runtime overhead
    // ============================================================

    "tagged_awaitable has zero size overhead"_test = []<typename AwaitableT>(std::type_identity<AwaitableT>) mutable {
            using raw_t          = AwaitableT;
            using tagged_t       = tagged_awaitable<test_tag, AwaitableT>;
            using pathological_t = tagged_awaitable<std::array<std::int32_t, 4>, AwaitableT>;

            // The wrapper should be exactly the size of the thing it wraps.
            expect(eq(sizeof(raw_t), sizeof(tagged_t)));
            expect(eq(sizeof(raw_t), sizeof(pathological_t)));
            // It should also share the same alignment requirements.
            expect(eq(alignof(raw_t), alignof(tagged_t)));
            expect(eq(alignof(raw_t), alignof(pathological_t)));
    } | std::array{
        std::type_identity<test_task<void>>{},
        std::type_identity<test_task<std::int32_t>>{},
        std::type_identity<test_task<std::array<std::int32_t, 4>>>{},

        std::type_identity<dummies::ready_awaiter<void>>{},
        std::type_identity<dummies::ready_awaiter<std::int32_t>>{},
        std::type_identity<dummies::ready_awaiter<std::array<std::int32_t, 4>>>{},

        std::type_identity<dummies::immediate_awaiter<void>>{},
        std::type_identity<dummies::immediate_awaiter<std::int32_t>>{},
        std::type_identity<dummies::immediate_awaiter<std::array<std::int32_t, 4>>>{},

        std::type_identity<dummies::adl::awaitable_by_adl<std::int32_t>>{},
        std::type_identity<dummies::adl::awaitable_by_adl<std::array<std::int32_t, 4>>>{},
    };

    // ============================================================
    // Await semantics
    // ============================================================

    "tagged_awaitable supports lvalue co_await"_test = [] mutable {
        coroutine_probe probe;
        constexpr std::int32_t expected = 42;

        auto coro = echo(expected);
        coro.get().set_probe(&probe);

        //NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines): This coroutine lambda is invoked and completed synchronously by the test. Their closure object therefore outlives the coroutine execution.
        const auto test = [&] -> test_task<std::int32_t> {
            const std::int32_t val = co_await coro;
            co_return val;
        };

        const auto result = run(test());
        expect(eq(result, expected));
        expect(eq(static_cast<std::int32_t>(probe.await_path), static_cast<std::int32_t>(coroutine_probe::path::lvalue)));
    };

    "tagged_awaitable supports rvalue co_await"_test = [] mutable {
        coroutine_probe probe;
        constexpr std::int32_t expected = 42;

        auto coro = echo(expected);
        coro.get().set_probe(&probe);

        //NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines): This coroutine lambda is invoked and completed synchronously by the test. Their closure object therefore outlives the coroutine execution.
        const auto test = [&] -> test_task<std::int32_t> {
            const std::int32_t val = co_await std::move(coro);
            co_return val;
        };

        const auto result = run(test());
        expect(eq(result, expected));
        expect(eq(static_cast<std::int32_t>(probe.await_path), static_cast<std::int32_t>(coroutine_probe::path::rvalue)));
    };

    "tagged_awaitable supports co_await by free function (ADL)"_test = [] mutable {
        constexpr std::int32_t expected = 42;

        using wrapper_t = tagged_awaitable<test_tag, dummies::adl::awaitable_by_adl<std::int32_t>>;
        // Note: This awaitable is trivially multi-shot because it doesn't require its own coroutine frame.
        const wrapper_t wrapped = dummies::adl::awaitable_by_adl{expected};

        expect(eq(run(as_task<std::int32_t>(wrapped)), expected));
    };

    "tagged_awaitable supports co_await of wrapped awaiter"_test = [] mutable {
        constexpr std::int32_t expected = 42;

        using wrapper_t = tagged_awaitable<test_tag, dummies::immediate_awaiter<std::int32_t>>;
        // Note: This awaitable is trivially multi-shot because it doesn't require its own coroutine frame.
        wrapper_t wrapped = dummies::immediate_awaiter{expected};

        expect(eq(run(as_task<std::int32_t>(wrapped)), expected));
        expect(eq(run(as_task<std::int32_t>(std::as_const(wrapped))), expected));
        expect(eq(run(as_task<std::int32_t>(wrapped)), expected));
    };

    // ============================================================
    // Coroutine probe lifecycle tracking
    // ============================================================

    "tagged_awaitable propagates coroutine lifecycle"_test = [] mutable {
        constexpr std::int32_t expected = 42;

        coroutine_probe probe;

        auto wrapped = echo(expected);
        wrapped.get().set_probe(&probe);

        //NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines): This coroutine lambda is invoked and completed synchronously by the test. Their closure object therefore outlives the coroutine execution.
        const auto test = [&] -> test_task<std::int32_t> { co_return co_await wrapped; };

        const auto result = run(test());

        expect(eq(result, expected));
        expect(that % probe.awaited);
        expect(that % probe.done);
    };

    "tagged_awaitable propagates destruction to underlying awaitable"_test = [] mutable {
        coroutine_probe probe;

        {
            //Wrap a task that hasn't started or finished
            auto wrapped = echo({});
            wrapped.get().set_probe(&probe);

            expect(that % !probe.done);
            expect(that % !probe.destroyed);
        } //`wrapped` goes out of scope here.

        //Verify the destruction worked
        expect(that % !probe.done);
        expect(thst % probe.destroyed);
    };

    // ============================================================
    // Nested awaitable composition
    // ============================================================

    "tagged_awaitable composes inside other coroutines"_test = [] mutable {
        constexpr std::int32_t base     = 7;
        constexpr std::int32_t mult     = 3;
        constexpr std::int32_t expected = base * mult;

        //NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines): This coroutine lambda is invoked and completed synchronously by the test. Their closure object therefore outlives the coroutine execution.
        const auto sub = [&] -> tagged_awaitable<test_tag, test_task<std::int32_t>> { co_return base; };

        //NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines): This coroutine lambda is invoked and completed synchronously by the test. Their closure object therefore outlives the coroutine execution.
        const auto main = [&] -> tagged_awaitable<test_tag, test_task<std::int32_t>> { co_return (co_await sub()) * mult; };

        const auto result = run(main());
        expect(eq(result, expected));
    };

    // ============================================================
    // Conversion to underlying awaitable
    // ============================================================

    "tagged_awaitable converts to underlying awaitable"_test = [] mutable {
        constexpr std::int32_t expected = 42;
        auto wrapped{echo(expected)};

        const auto run_underlying = [](test_task<std::int32_t> task) { return run(task); };

        const auto result = run_underlying(std::move(wrapped));
        expect(eq(result, expected));
    };

    // ============================================================
    // Coroutine Traits promise type
    // ============================================================

    "tagged_awaitable preserves underlying awaitable's promise type"_test = [] mutable {
        expect(that % std::same_as<test_task<std::int32_t>::promise_type, std::coroutine_traits<tagged_awaitable<test_tag, test_task<std::int32_t>>>::promise_type>));
        expect(that % std::same_as<test_task<void>::promise_type, std::coroutine_traits<tagged_awaitable<test_tag, test_task<void>>>::promise_type>);
    };

    "tagged_awaitable usable as coroutine return type"_test = [] mutable {
        const auto tagged_echo_coro = [](std::int32_t value) -> tagged_awaitable<test_tag, test_task<std::int32_t>> {
            co_return value;
        };

        constexpr std::int32_t expected = 42;

        const auto result = run(tagged_echo_coro(expected));

        expect(eq(result, expected));
    };

    // ============================================================
    // Exception Propagation
    // ============================================================

    "tagged_awaitable propagates exceptions"_test = [] mutable {
        const auto wrapped = [](std::int32_t value) -> tagged_awaitable<test_tag, test_task<std::int32_t>> {
            throw std::runtime_error("boom");
            co_return value;
        };

        constexpr std::int32_t herring  = 42;
        constexpr std::int32_t expected = 0;

        std::int32_t result = expected;

        expect(throws<std::runtime_error>([&]{ result = run(wrapped(herring)); }));

        expect(eq(result, expected));
    };
}
