// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
// Integration tests for net.telnet:awaitables (coroutine execution with harness)

//Including Asio via preprocessor until importable header units are reliable.
#include <asio.hpp>

import net.telnet;
import net.telnet.test_support;
import ut;
import std;

using namespace ut;
using namespace net::telnet::awaitables;
using namespace net::telnet::test_support::coroutine_harness;

// Trivial awaitable that suspends once and immediately resumes via symmetric transfer
struct immediate_suspend_resume {
    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) noexcept
    {
        return caller; // symmetric transfer → resume caller right away
    }

    constexpr void await_resume() const noexcept {}
};

suite telnet_awaitables_integration_tests = [] mutable {
    // ────────────────────────────────────────────────────────────────
    // Tests wrapping asio::awaitable<T>
    // ────────────────────────────────────────────────────────────────

    "option_enablement_awaitable immediate completion (asio)"_test = [] mutable {
        coroutine_probe probe;

        auto task = []() -> test_task<void> {
            option_enablement_awaitable aw = []() -> asio::awaitable<void> { co_return; }();

            co_await aw;
        }();

        task.set_probe(&probe);
        run(task);

        expect(eq(probe.awaited, true));
        expect(eq(probe.suspended, false));
        expect(eq(probe.resumed, true));
        expect(eq(probe.done, true));
        expect(eq(probe.destroyed, true));
    };

    "subnegotiation_awaitable returns expected tuple (asio)"_test = [] mutable {
        coroutine_probe probe;

        auto task = [&]() -> test_task<std::size_t> {
            subnegotiation_awaitable aw = []() -> asio::awaitable<std::tuple<option, std::vector<byte_t>>> {
                option opt{option::id_num::echo, "Echo"};
                std::vector<byte_t> payload{0x01, 0x02, 0x03};
                co_return {opt, std::move(payload)};
            }();

            auto [opt, data] = co_await aw;
            co_return data.size();
        }();

        task.set_probe(&probe);
        auto size = run(task);

        expect(eq(size, 3uz));
        expect(eq(probe.awaited, true));
        expect(eq(probe.suspended, false));
        expect(eq(probe.resumed, true));
        expect(eq(probe.done, true));
    };

    "tagged awaitable suspends and resumes via symmetric transfer (asio)"_test = [] mutable {
        coroutine_probe probe;

        auto task = []() -> test_task<int> {
            tagged_awaitable<tags::option_disablement_tag, int> tagged_aw = []() -> asio::awaitable<int> {
                co_await immediate_suspend_resume{};
                co_return 777;
            }();

            int value = co_await std::move(tagged_aw);
            co_return value;
        }();

        task.set_probe(&probe);
        auto result = run(task);

        expect(eq(result, 777));
        expect(eq(probe.awaited, true));
        expect(eq(probe.suspended, true));
        expect(eq(probe.resumed, true));
        expect(eq(probe.done, true));
    };

    "co_await on lvalue vs rvalue tagged awaitable (asio)"_test = [] mutable {
        coroutine_probe probe;

        auto task = [&]() -> test_task<std::pair<int, int>> {
            auto make = []() -> asio::awaitable<int> {
                co_await immediate_suspend_resume{};
                co_return 123;
            };

            // lvalue co_await
            tagged_awaitable<tags::subnegotiation_tag, int> ta_l = make();
            int lv                                               = co_await ta_l;

            // rvalue co_await
            int rv = co_await make();

            co_return {lv, rv};
        }();

        task.set_probe(&probe);
        auto [lv_result, rv_result] = run(task);

        expect(eq(lv_result, 123));
        expect(eq(rv_result, 123));

        expect(eq(probe.awaited, true));
        expect(eq(probe.suspended, true));
        expect(eq(probe.resumed, true));
        expect(eq(probe.done, true));
    };

    // ────────────────────────────────────────────────────────────────
    // Tests directly wrapping test_task<T>
    // ────────────────────────────────────────────────────────────────

    "tagged_awaitable wrapping trivial test_task"_test = [] mutable {
        coroutine_probe inner_probe;

        auto inner_task = []() -> test_task<int> { co_return 42; }();
        inner_task.set_probe(&inner_probe);

        tagged_awaitable<tags::option_enablement_tag, int> wrapped{std::move(inner_task)};

        coroutine_probe outer_probe;
        auto outer_task = [&]() -> test_task<int> {
            int value = co_await wrapped;
            co_return value;
        }();
        outer_task.set_probe(&outer_probe);

        auto result = run(outer_task);

        expect(eq(result, 42));

        // Inner (wrapped) task
        expect(eq(inner_probe.awaited, false));
        expect(eq(inner_probe.done, true));
        expect(eq(inner_probe.destroyed, true));

        // Outer task
        expect(eq(outer_probe.awaited, true));
        expect(eq(outer_probe.suspended, false));
        expect(eq(outer_probe.resumed, true));
        expect(eq(outer_probe.done, true));
    };

    "tagged_awaitable wrapping test_task with suspension"_test = [] mutable {
        coroutine_probe inner_probe;

        auto inner_task = []() -> test_task<int> {
            co_await immediate_suspend_resume{};
            co_return 99;
        }();
        inner_task.set_probe(&inner_probe);

        tagged_awaitable<tags::subnegotiation_tag, int> wrapped{std::move(inner_task)};

        coroutine_probe outer_probe;
        auto outer_task = [&]() -> test_task<int> {
            int value = co_await wrapped;
            co_return value;
        }();
        outer_task.set_probe(&outer_probe);

        auto result = run(outer_task);

        expect(eq(result, 99));

        // Inner probe: suspension inside the wrapped task
        expect(eq(inner_probe.awaited, true));
        expect(eq(inner_probe.suspended, true));
        expect(eq(inner_probe.resumed, true));
        expect(eq(inner_probe.done, true));
        expect(eq(inner_probe.destroyed, true));

        // Outer probe: sees suspension because it awaited something that suspended
        expect(eq(outer_probe.awaited, true));
        expect(eq(outer_probe.suspended, true));
        expect(eq(outer_probe.resumed, true));
        expect(eq(outer_probe.done, true));
    };

    "move semantics when wrapping moved test_task"_test = [] mutable {
        coroutine_probe inner_probe;

        auto make_inner = []() -> test_task<int> {
            co_await immediate_suspend_resume{};
            co_return 123;
        };

        auto inner_task = make_inner();
        inner_task.set_probe(&inner_probe);

        tagged_awaitable<tags::option_disablement_tag, int> wrapped{std::move(inner_task)};

        coroutine_probe outer_probe;
        auto outer_task = [&]() -> test_task<int> {
            int value = co_await std::move(wrapped);
            co_return value;
        }();
        outer_task.set_probe(&outer_probe);

        auto result = run(outer_task);

        expect(eq(result, 123));

        // Inner task moved into wrapper
        expect(eq(inner_probe.moved, true));
        expect(eq(inner_probe.awaited, true));
        expect(eq(inner_probe.suspended, true));
        expect(eq(inner_probe.resumed, true));
        expect(eq(inner_probe.done, true));
    };

    "tag type safety prevents wrong tag assignment"_test = [] mutable {
        auto inner = []() -> test_task<void> { co_return; }();

        tagged_awaitable<tags::option_enablement_tag, void> good{std::move(inner)};

        // These should not compile (verified via static_assert)
        static_assert(!std::is_convertible_v<
                      tagged_awaitable<tags::option_enablement_tag, void>,
                      tagged_awaitable<tags::option_disablement_tag, void>
        >);

        static_assert(!std::is_convertible_v<
                      tagged_awaitable<tags::subnegotiation_tag, void>,
                      tagged_awaitable<tags::option_enablement_tag, void>
        >);

        // Minimal runtime execution to keep test valid
        coroutine_probe probe;
        auto outer = [&]() -> test_task<void> { co_await good; }();
        outer.set_probe(&probe);
        run(outer);
    };
};

int main() {}
