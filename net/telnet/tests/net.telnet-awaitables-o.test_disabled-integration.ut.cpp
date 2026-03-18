// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:awaitables with asio::awaitable

#include <asio.hpp>

import net.telnet;
import ut;
import std;

using namespace ut;
using namespace net::telnet::awaitables;

suite net_telnet_awaitables_asio_integration_tests = [] mutable {
    // ============================================================
    // Basic wrapping of asio::awaitable
    // ============================================================

    "tagged_awaitable forwards result correctly"_test = [] mutable {
        asio::io_context ctx;

        tagged_awaitable<tags::option_enablement_tag, int> wrapped = []() mutable -> asio::awaitable<int> { co_return 42; }();

        auto fut = asio::co_spawn(
            ctx,
            [wrapped = std::move(wrapped)]() mutable -> asio::awaitable<int> { co_return co_await std::move(wrapped).get(); },
            asio::as_tuple(asio::use_future)
        );

        ctx.run();

        auto [_, res] = fut.get();
        expect(eq(res, 42));
    };

    // ============================================================
    // Implicit conversion from asio::awaitable
    // ============================================================

    "asio awaitable implicitly converts to tagged_awaitable"_test = [] mutable {
        asio::io_context ctx;

        tagged_awaitable<tags::option_enablement_tag, int> wrapped = []() mutable -> asio::awaitable<int> { co_return 77; }();

        auto fut = asio::co_spawn(ctx, std::move(wrapped).get(), asio::as_tuple(asio::use_future));

        ctx.run();

        auto [_, res] = fut.get();
        expect(eq(res, 77));
    };

    // ============================================================
    // Rvalue co_await support
    // ============================================================

    "rvalue tagged_awaitable co_await works"_test = [] mutable {
        asio::io_context ctx;

        auto fut = asio::co_spawn(
            ctx,
            [&]() mutable -> asio::awaitable<int> {
                tagged_awaitable<tags::option_enablement_tag, int> a{[]() mutable -> asio::awaitable<int> { co_return 5; }()};
                co_return co_await std::move(a).get();
            },
            asio::as_tuple(asio::use_future)
        );

        ctx.run();

        auto [_, res] = fut.get();
        expect(eq(res, 5));
    };

    // ============================================================
    // Nested coroutine composition
    // ============================================================

    "tagged_awaitable composes inside nested coroutines"_test = [] mutable {
        asio::io_context ctx;

        auto leaf = []() mutable -> asio::awaitable<int> { co_return 9; };

        tagged_awaitable<tags::option_enablement_tag, int> wrapped_leaf = [&]() mutable -> asio::awaitable<int> {
            co_return co_await leaf();
        }();

        auto fut = asio::co_spawn(ctx, std::move(wrapped_leaf).get(), asio::as_tuple(asio::use_future));

        ctx.run();

        auto [_, res] = fut.get();
        expect(eq(res, 9));
    };

    // ============================================================
    // Void awaitables
    // ============================================================

    "option_enablement_awaitable works with void coroutine"_test = [] mutable {
        asio::io_context ctx;

        bool executed = false;

        option_enablement_awaitable wrapped = [&]() mutable -> asio::awaitable<void> {
            executed = true;
            co_return;
        }();

        auto fut = asio::co_spawn(ctx, std::move(wrapped).get(), asio::as_tuple(asio::use_future));

        ctx.run();
        (void)fut.get();

        expect(eq(executed, true));
    };

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

    // ============================================================
    // Executor propagation correctness
    // ============================================================

    "tagged_awaitable preserves executor context"_test = [] mutable {
        asio::io_context ctx;

        bool ran_on_executor = false;

        tagged_awaitable<tags::option_enablement_tag, int> wrapped = [&]() mutable -> asio::awaitable<int> {
            auto ex = co_await asio::this_coro::executor;
            (void)ex;

            ran_on_executor = true;
            co_return 123;
        }();

        auto fut = asio::co_spawn(ctx, std::move(wrapped).get(), asio::as_tuple(asio::use_future));

        ctx.run();

        auto [_, res] = fut.get();
        expect(eq(ran_on_executor, true));
        expect(eq(res, 123));
    };

    // ============================================================
    // Deep coroutine chaining (symmetric transfer behavior)
    // ============================================================

    "tagged_awaitable composes across coroutine layers"_test = [] mutable {
        asio::io_context ctx;

        auto leaf = []() mutable -> asio::awaitable<int> { co_return 10; };

        tagged_awaitable<tags::option_enablement_tag, int> middle = [&]() mutable -> asio::awaitable<int> {
            int v = co_await leaf();
            co_return v + 5;
        }();

        auto top = [&]() mutable -> asio::awaitable<int> {
            int v = co_await std::move(middle).get();
            co_return v * 2;
        };

        auto fut = asio::co_spawn(ctx, top(), asio::as_tuple(asio::use_future));

        ctx.run();

        auto [_, res] = fut.get();
        expect(eq(res, 30));
    };
};

int main() {}
