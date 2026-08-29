// SPDX-License-Identifier: Apache-2.0
// Unit tests for tools.test.coroutine_harness:dummies

import tools.test.coroutine_harness;

import ut;
import std;

using namespace ut;
using namespace tools::test::coroutine_harness;

namespace {
template<typename AwaiterT, typename T>
concept const_lvalue_resumable = requires(T& result, AwaiterT& awaiter) { result = std::as_const(awaiter).await_resume(); };

//NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
suite dummy_awaitable_tests = [] mutable {
    // ============================================================
    // trivial_awaiter_base
    // ============================================================

    "trivial_awaiter_base returns stored value"_test = [] mutable {
        constexpr int expected = 42;
        dummies::trivial_awaiter_base<int> base{expected};
        expect(eq(base.await_resume(), expected));
    };

    "trivial_awaiter_base<void> is no-op"_test = [] mutable {
        dummies::trivial_awaiter_base<void> base;

        bool threw = false;
        try {
            base.await_resume();
        } catch (...) {
            threw = true;
        }
        expect(eq(threw, false));
        expect(eq(std::is_void_v<decltype(base.await_resume())>, true));
    };

    "trivial_awaiter_base supports move-only return types (unique_ptr)"_test = [] mutable {
        constexpr int expected = 42;
        dummies::trivial_awaiter_base<std::unique_ptr<int>> base{std::make_unique<int>(expected)};

        const auto result = base.await_resume();

        expect(eq(static_cast<bool>(result), true));
        if (result) {
            expect(eq(*result, expected));
        }
    };

    "trivial_awaiter_base await_resume value category correctness"_test = [] mutable {
        bool lvalue = requires(int& result, dummies::trivial_awaiter_base<int>& base) { result = base.await_resume(); };
        bool rvalue = requires(int& result, dummies::trivial_awaiter_base<int>& base) {
                          result = std::move(base).await_resume();
                      };
        bool const_lvalue = const_lvalue_resumable<dummies::trivial_awaiter_base<int>, int>;
        bool clvalue_move = const_lvalue_resumable<dummies::trivial_awaiter_base<std::unique_ptr<int>>, std::unique_ptr<int>>;

        expect(eq(const_lvalue, true));
        expect(eq(lvalue, true));
        expect(eq(rvalue, true));
        expect(eq(clvalue_move, false));
    };

    // ============================================================
    // ready_awaiter
    // ============================================================

    "ready_awaiter await_suspend throws (contract enforcement)"_test = [] mutable {
        dummies::ready_awaiter<int> awaiter{};

        bool threw = false;
        try {
            awaiter.await_suspend(std::noop_coroutine());
        } catch (const std::logic_error&) {
            threw = true;
        }
        expect(eq(threw, true));
    };

    "ready_awaiter returns value without suspension"_test = [] mutable {
        int expected = 42;

        coroutine_probe probe;

        auto task = as_task<int>(dummies::ready_awaiter{expected});
        task.set_probe(&probe);

        const auto result = run(task);

        expect(eq(result, expected));
        expect(eq(probe.suspended, false));
    };

    "ready_awaiter<void> completes without suspension"_test = [] mutable {
        coroutine_probe probe;

        auto task = as_task<void>(dummies::ready_awaiter<void>{});
        task.set_probe(&probe);

        run(task);

        expect(eq(probe.suspended, false));
    };

    // ============================================================
    // immediate_awaiter
    // ============================================================

    "immediate_awaiter suspends and resumes"_test = [] mutable {
        int expected = 7;

        coroutine_probe probe;

        auto task = as_task<int>(dummies::immediate_awaiter{expected});

        task.set_probe(&probe);

        const auto result = run(task);

        expect(eq(result, expected));
        expect(eq(probe.suspended, true));
        expect(eq(probe.done, true));
    };

    "immediate_awaiter<void> suspends and resumes"_test = [] mutable {
        coroutine_probe probe;

        auto task = as_task<void>(dummies::immediate_awaiter<void>{});

        task.set_probe(&probe);

        run(task);

        expect(eq(probe.suspended, true));
        expect(eq(probe.done, true));
    };

    "immediate_awaiter fallback path (non-test_task)"_test = [] mutable {
        dummies::immediate_awaiter<void> awaiter{};

        auto coro         = std::noop_coroutine();
        const auto result = awaiter.await_suspend(coro);

        // This confirms immediate symmetric transfer back to the caller.
        expect(eq(result.address(), coro.address()));
    };

    // ============================================================
    // ADL awaitable
    // ============================================================

    "adl awaitable resolves via operator co_await"_test = [] mutable {
        int expected = 42;

        coroutine_probe probe;

        auto task = as_task<int>(dummies::adl::awaitable_by_adl<int>{expected});

        task.set_probe(&probe);

        const auto result = run(task);

        expect(eq(result, expected));
        expect(eq(probe.suspended, false)); // uses ready_awaiter
    };
};
} //namespace

int main() {}
