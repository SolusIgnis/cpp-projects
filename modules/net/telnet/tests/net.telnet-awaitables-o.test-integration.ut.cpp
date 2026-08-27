// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:awaitables with asio::awaitable

#include <asio.hpp>

import net.telnet;
import tools.test.coroutine_harness;
import framework.coroutines.tagged_awaitable;
import ut;
import std;

using namespace ut;
using namespace net::telnet::awaitables;
using framework::coroutines::tagged_awaitable;
using namespace tools::test::coroutine_harness;

struct test_tag;
using test_wrapper_int  = tagged_awaitable<test_tag, asio::awaitable<int>>;
using test_wrapper_void = tagged_awaitable<test_tag, asio::awaitable<void>>;

asio::awaitable<int> echo(int value)
{
    co_return value;
}

test_wrapper_int tagged_echo(int value)
{
    co_return value;
}

suite net_telnet_awaitables_integration_tests = [] mutable {
    // ============================================================
    // Basic constructibility
    // ============================================================

    "telnet::awaitables tagged_awaitable instances default constructible"_test = [] mutable {
        expect(eq(std::default_initializable<option_enablement_awaitable>, true));
        expect(eq(std::default_initializable<option_disablement_awaitable>, true));
        expect(eq(std::default_initializable<subnegotiation_awaitable>, true));
    };

    // ============================================================
    // Type Safety and Tag Isolation
    // ============================================================

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
    // Basic wrapping of asio::awaitable
    // ============================================================

    "tagged_awaitable (un)wraps on assignment to/from asio::awaitable"_test = [] mutable {
        int expected = 42;

        test_wrapper_int wrapped       = echo(expected);
        asio::awaitable<int> unwrapped = std::move(wrapped);

        asio::io_context ctx;

        auto fut = asio::co_spawn(ctx, std::move(unwrapped), asio::use_future);

        ctx.run();

        expect(eq(fut.get(), expected));
    };

    "tagged_awaitable forwards result correctly"_test = [] mutable {
        int expected = 42;

        asio::io_context ctx;

        auto fut = asio::co_spawn(ctx, tagged_echo(expected).get(), asio::use_future);

        ctx.run();

        expect(eq(fut.get(), expected));
    };

    // ============================================================
    // Rvalue co_await support
    // ============================================================

    "rvalue tagged_awaitable co_await works"_test = [] mutable {
        int expected = 5;

        asio::io_context ctx;

        auto fut = asio::co_spawn(
            ctx,
            [expected] mutable -> asio::awaitable<int> {
                test_wrapper_int wrapped{echo(expected)};
                co_return co_await std::move(wrapped).get();
            },
            asio::use_future
        );

        ctx.run();

        expect(eq(fut.get(), expected));
    };

    // ============================================================
    // Nested coroutine composition
    // ============================================================

    "tagged_awaitable composes inside nested coroutines"_test = [] mutable {
        int expected = 9;

        asio::io_context ctx;

        auto leaf = echo(expected);

        auto wrap_leaf    = [&] mutable -> test_wrapper_int { co_return co_await std::move(leaf); };
        auto wrapped_leaf = wrap_leaf();

        auto fut = asio::co_spawn(ctx, std::move(wrapped_leaf).get(), asio::use_future);

        ctx.run();

        expect(eq(fut.get(), expected));
    };

    // ============================================================
    // Void awaitables
    // ============================================================

    "tagged_awaitable works with void coroutine"_test = [] mutable {
        asio::io_context ctx;

        bool executed = false;

        auto wrap = [&] mutable -> asio::awaitable<void> {
            executed = true;
            co_return;
        };

        test_wrapper_void wrapped = wrap();

        auto fut = asio::co_spawn(ctx, std::move(wrapped).get(), asio::use_future);

        ctx.run();

        expect(eq(executed, true));
    };

    // ============================================================
    // Subnegotiation return type propagation
    // ============================================================

    "subnegotiation_awaitable returns tuple correctly"_test = [] mutable {
        asio::io_context ctx;

        using net::telnet::option;
        using net::telnet::byte_t;

        auto wrap = [] mutable -> subnegotiation_awaitable {
            co_return std::tuple<option, std::vector<byte_t>>{
                option::id_num::echo, std::vector<byte_t>{1, 2, 3}
            };
        };

        subnegotiation_awaitable wrapped = wrap();

        auto fut = asio::co_spawn(
            ctx,
            [wrapped = std::move(wrapped)] mutable -> asio::awaitable<std::optional<std::tuple<option, std::vector<byte_t>>>> {
                co_return std::optional{co_await std::move(wrapped).get()};
            },
            asio::as_tuple(asio::use_future)
        );

        ctx.run();

        auto [_, result] = fut.get();

        expect(eq(result.has_value(), true));
        expect(eq(std::get<1>(*result).size(), std::size_t{3}));
    };

    // ============================================================
    // Executor propagation correctness
    // ============================================================

    "tagged_awaitable preserves executor context"_test = [] mutable {
        bool ran_on_executor = false;
        int expected         = 123;

        asio::io_context ctx;

        auto wrap = [&] mutable -> test_wrapper_int {
            auto exec = co_await asio::this_coro::executor;

            ran_on_executor = (exec == ctx.get_executor());
            co_return expected;
        };

        auto wrapped = wrap();

        auto fut = asio::co_spawn(ctx, std::move(wrapped).get(), asio::use_future);

        ctx.run();

        expect(eq(ran_on_executor, true));
        expect(eq(fut.get(), expected));
    };

    // ============================================================
    // Deep coroutine chaining (symmetric transfer behavior)
    // ============================================================

    "tagged_awaitable composes across coroutine layers"_test = [] mutable {
        int start    = 10;
        int inc      = 5;
        int mult     = 2;
        int expected = (start + inc) * mult;

        bool ran_on_executor = false;

        asio::io_context ctx;

        auto leaf = echo(start);

        auto make_middle = [&] mutable -> test_wrapper_int {
            int res         = co_await std::move(leaf);
            auto exec       = co_await asio::this_coro::executor;
            ran_on_executor = (exec == ctx.get_executor());
            co_return res + inc;
        };

        auto middle = make_middle();

        auto top = [&] mutable -> asio::awaitable<int> {
            int res = co_await std::move(middle).get();
            co_return res* mult;
        };

        auto fut = asio::co_spawn(ctx, top(), asio::use_future);

        ctx.run();

        expect(eq(ran_on_executor, true));
        expect(eq(fut.get(), expected));
    };
};

int main() {}
