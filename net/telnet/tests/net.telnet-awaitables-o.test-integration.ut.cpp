// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:awaitables with asio::awaitable

import net.telnet;
import ut;
import std;

//Including Asio via preprocessor until importable header units are reliable.
#include <asio.hpp>

using namespace ut;
using namespace net::telnet::awaitables;

suite net_telnet_awaitables_asio_integration_tests = [] mutable {
    // ============================================================
    // Basic wrapping of asio::awaitable
    // ============================================================

    "tagged_awaitable wraps asio::awaitable and forwards result"_test = [] mutable {
        asio::io_context ctx;

        auto make = []() -> asio::awaitable<int> { co_return 42; };

        tagged_awaitable<tags::option_enablement_tag, int> wrapped{make()};

        int result = 0;

        asio::co_spawn(
            ctx,
            [&]() -> asio::awaitable<void> {
                result = co_await wrapped;
                co_return;
            },
            asio::detached
        );

        ctx.run();

        expect(eq(result, 42));
    };

    // ============================================================
    // Implicit conversion from asio::awaitable
    // ============================================================

    "asio awaitable implicitly converts to tagged_awaitable"_test = [] mutable {
        asio::io_context ctx;

        auto producer = []() -> asio::awaitable<int> { co_return 77; };

        tagged_awaitable<tags::option_enablement_tag, int> wrapped = producer();

        int result = 0;

        asio::co_spawn(
            ctx,
            [&]() -> asio::awaitable<void> {
                result = co_await wrapped;
                co_return;
            },
            asio::detached
        );

        ctx.run();

        expect(eq(result, 77));
    };

    // ============================================================
    // Rvalue co_await support
    // ============================================================

    "rvalue tagged_awaitable co_await works with asio"_test = [] mutable {
        asio::io_context ctx;

        auto producer = []() -> asio::awaitable<int> { co_return 5; };

        int result = 0;

        asio::co_spawn(
            ctx,
            [&]() -> asio::awaitable<void> {
                tagged_awaitable<tags::option_enablement_tag, int> a{producer()};
                result = co_await std::move(a);
                co_return;
            },
            asio::detached
        );

        ctx.run();

        expect(eq(result, 5));
    };

    // ============================================================
    // Nested coroutine composition
    // ============================================================

    "tagged_awaitable composes in nested asio coroutines"_test = [] mutable {
        asio::io_context ctx;

        auto leaf = []() -> asio::awaitable<int> { co_return 9; };

        auto wrapped_leaf = [&]() -> tagged_awaitable<tags::option_enablement_tag, int> { co_return co_await leaf(); };

        int result = 0;

        asio::co_spawn(
            ctx,
            [&]() -> asio::awaitable<void> {
                result = co_await wrapped_leaf();
                co_return;
            },
            asio::detached
        );

        ctx.run();

        expect(eq(result, 9));
    };

    // ============================================================
    // Void awaitables
    // ============================================================

    "option_enablement_awaitable works with void returning asio coroutine"_test = [] mutable {
        asio::io_context ctx;

        bool executed = false;

        auto producer = [&]() -> asio::awaitable<void> {
            executed = true;
            co_return;
        };

        option_enablement_awaitable wrapped{producer()};

        asio::co_spawn(
            ctx,
            [&]() -> asio::awaitable<void> {
                co_await wrapped;
                co_return;
            },
            asio::detached
        );

        ctx.run();

        expect(eq(executed, true));
    };

    // ============================================================
    // Subnegotiation return type propagation
    // ============================================================

    "subnegotiation_awaitable returns tuple correctly"_test = [] mutable {
        asio::io_context ctx;

        using net::telnet::option;
        using byte_t = std::uint8_t;

        auto producer = []() -> asio::awaitable<std::tuple<option, std::vector<byte_t>>> {
            co_return std::tuple<option, std::vector<byte_t>>{
                static_cast<option>(1), std::vector<byte_t>{1, 2, 3}
            };
        };

        subnegotiation_awaitable wrapped{producer()};

        std::tuple<option, std::vector<byte_t>> result;

        asio::co_spawn(
            ctx,
            [&]() -> asio::awaitable<void> {
                result = co_await wrapped;
                co_return;
            },
            asio::detached
        );

        ctx.run();

        expect(eq(std::get<1>(result).size(), std::size_t{3}));
    };

    // ============================================================
    // Symmetric coroutine transfer / deep chaining
    // ============================================================

    "tagged_awaitable composes across multiple coroutine layers"_test = [] mutable {
        asio::io_context ctx;

        auto leaf = []() -> asio::awaitable<int> { co_return 10; };

        auto middle = [&]() -> tagged_awaitable<tags::option_enablement_tag, int> {
            int v = co_await leaf();
            co_return v + 5;
        };

        auto top = [&]() -> asio::awaitable<int> {
            int v = co_await middle();
            co_return v * 2;
        };

        int result = 0;

        asio::co_spawn(
            ctx,
            [&]() -> asio::awaitable<void> {
                result = co_await top();
                co_return;
            },
            asio::detached
        );

        ctx.run();

        expect(eq(result, 30));
    };
};

int main() {}
