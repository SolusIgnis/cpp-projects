// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:awaitables with asio::awaitable

#include <asio.hpp>

import net.telnet;
import ut;
import std;

using namespace ut;
using namespace net::telnet::awaitables;

struct test_tag {};
using test_wrapper_int = tagged_awaitable<test_tag, int>;
using test_wrapper_void = tagged_awaitable<test_tag, void>;

asio::awaitable<int> echo(int value)
{
    co_return value;
}

suite net_telnet_awaitables_asio_integration_tests = [] mutable {
    // ============================================================
    // Basic wrapping of asio::awaitable
    // ============================================================

    "tagged_awaitable forwards result correctly"_test = [] mutable {
        int expected = 42;

        asio::io_context ctx;

        test_wrapper_int wrapped = echo(expected);

        auto fut = asio::co_spawn(
            ctx,
            std::move(wrapped).get(),
            asio::use_future
        );

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
            [expected]() mutable -> asio::awaitable<int> {
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

        test_wrapper_int wrapped_leaf = [&]() mutable -> asio::awaitable<int> {
            co_return co_await std::move(leaf);
        }();

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

        test_wrapper_void wrapped = [&]() mutable -> asio::awaitable<void> {
            executed = true;
            co_return;
        }();

        auto fut = asio::co_spawn(ctx, std::move(wrapped).get(), asio::use_future);

        ctx.run();

        expect(eq(executed, true));
    };
#if 0
    // ============================================================
    // Subnegotiation return type propagation
    // ============================================================

    "subnegotiation_awaitable returns tuple correctly"_test = [] mutable {
        asio::io_context ctx;

        using net::telnet::option;
        using net::telnet::byte_t;

        subnegotiation_awaitable wrapped = []() mutable -> asio::awaitable<std::tuple<option, std::vector<byte_t>>> {
            co_return std::tuple<option, std::vector<byte_t>>{
                static_cast<option::id_num>(1), std::vector<byte_t>{1, 2, 3}
            };
        }();

        auto fut = asio::co_spawn(
            ctx,
            [wrapped = std::move(wrapped)]() mutable
                -> asio::awaitable<std::optional<std::tuple<option, std::vector<byte_t>>>> {
                co_return co_await std::move(wrapped).get();
            },
           asio::as_tuple(asio::use_future)
        );

        ctx.run();

        auto [_, result] = fut.get();
        
        expect(eq(result.has_value(), true));
        expect(eq(std::get<1>(*result).size(), std::size_t{3}));
    };
#endif
    // ============================================================
    // Executor propagation correctness
    // ============================================================

    "tagged_awaitable preserves executor context"_test = [] mutable {
        bool ran_on_executor = false;
        int expected = 123;

        asio::io_context ctx;

        test_wrapper_int wrapped = [&]() mutable -> asio::awaitable<int> {
            auto exec = co_await asio::this_coro::executor;

            ran_on_executor = (exec == ctx.get_executor());
            co_return expected;
        }();

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

        test_wrapper_int middle = [&]() mutable -> asio::awaitable<int> {
            int res = co_await std::move(leaf);
            auto exec = co_await asio::this_coro::executor;
            ran_on_executor = (exec == ctx.get_executor());
            co_return res + inc;
        }();

        auto top = [&]() mutable -> asio::awaitable<int> {
            int res = co_await std::move(middle).get();
            co_return res * mult;
        };

        auto fut = asio::co_spawn(ctx, top(), asio::use_future);

        ctx.run();

        expect(eq(ran_on_executor, true));
        expect(eq(fut.get(), expected));
    };
};

int main() {}
