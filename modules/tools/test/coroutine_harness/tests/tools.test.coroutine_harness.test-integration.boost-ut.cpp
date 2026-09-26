// SPDX-License-Identifier: Apache-2.0
// Integration tests for tools.test.coroutine_harness

import tools.test.coroutine_harness;

import boost.ut;
import std;

using namespace boost::ext::ut;
using namespace tools::test::coroutine_harness;

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    "mega coroutine harness: full integration"_test = [] mutable {
        constexpr std::int32_t first   = 1;
        constexpr std::int32_t second  = 2;
        constexpr std::int32_t third   = 3;
        constexpr std::int32_t fourth  = 4;
        constexpr std::int32_t fifth   = 5;
        constexpr std::int32_t sixth   = 6;
        constexpr std::int32_t seventh = 7;

        constexpr std::int32_t input_l   = 42;
        constexpr std::int32_t input_r   = 58;
        constexpr std::int32_t input_ptr = 99;
        constexpr std::int32_t expected  = input_l + input_r + input_ptr;

        std::vector<std::int32_t> trace;

        coroutine_probe probe_int_lvalue;
        coroutine_probe probe_int_rvalue;
        coroutine_probe probe_ptr;
        coroutine_probe probe_void;
        coroutine_probe probe_throw;
        coroutine_probe probe_nested;

        //NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
        // These coroutine lambdas are invoked and completed synchronously by the test harness.
        // Their closure objects therefore outlive the coroutine execution.

        // -------------------------------
        // Leaf tasks
        // -------------------------------

        // int via ready_awaiter (lvalue)
        auto leaf_int_l = [&] -> test_task<std::int32_t> {
            trace.push_back(second);
            co_return run(as_task<std::int32_t>(dummies::ready_awaiter{input_l}));
        };

        // int via ready_awaiter (rvalue)
        auto leaf_int_r = [&] -> test_task<std::int32_t> {
            trace.push_back(third);
            co_return run(as_task<std::int32_t>(dummies::ready_awaiter{input_r}));
        };

        // unique_ptr via immediate_awaiter
        auto leaf_ptr = [&] -> test_task<std::unique_ptr<std::int32_t>> {
            trace.push_back(fourth);
            auto awaiter = dummies::immediate_awaiter{std::make_unique<std::int32_t>(input_ptr)};
            co_return run(as_task<std::unique_ptr<std::int32_t>>(std::move(awaiter)));
        };

        // void via immediate_awaiter
        auto leaf_void = [&] -> test_task<void> {
            trace.push_back(fifth);
            co_await as_task<void>(dummies::immediate_awaiter<void>{});
        };

        // throwing awaiter
        //NOLINTBEGIN(readability-convert-member-functions-to-static): Awaiter protocol.
        auto leaf_throw = [&] -> test_task<void> {
            trace.push_back(sixth);
            struct throwing_awaiter {
                [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }
                void await_suspend(std::coroutine_handle<> /*unused*/) const noexcept {}
                [[noreturn]] void await_resume() const { throw std::runtime_error("boom"); }
            };
            co_await as_task<void>(throwing_awaiter{});
        };
        //NOLINTEND(readability-convert-member-functions-to-static)

        // -------------------------------
        // Nested composition task
        // -------------------------------
        const auto make_nested = [&] -> test_task<std::int32_t> {
            trace.push_back(first);

            const std::int32_t val_l = co_await leaf_int_l().set_probe(&probe_int_lvalue);
            const std::int32_t val_r = co_await std::move(leaf_int_r().set_probe(&probe_int_rvalue));
            const auto ptr           = co_await leaf_ptr().set_probe(&probe_ptr);
            co_await leaf_void().set_probe(&probe_void); // void task

            // Exception propagation check
            bool threw           = false;
            bool wrong_exception = false;
            try {
                co_await leaf_throw().set_probe(&probe_throw);
            } catch (const std::runtime_error&) {
                threw = true;
            } catch (...) {
                wrong_exception = true;
            }
            expect(that % threw);
            expect(that % !wrong_exception);

            trace.push_back(seventh);
            co_return val_l + val_r + *ptr;
        };

        auto nested = make_nested();
        nested.set_probe(&probe_nested);
        //NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)

        // -------------------------------
        // Run mega-task
        // -------------------------------
        const auto result = run(nested);

        // -------------------------------
        // Verify result
        // -------------------------------
        expect(eq(result, expected));

        // -------------------------------
        // Verify trace order
        // -------------------------------
        const std::vector<std::int32_t> expected_trace{first, second, third, fourth, fifth, sixth, seventh};

        expect(that % trace == expected_trace);

        // -------------------------------
        // Verify probes for all leaf tasks
        // -------------------------------

        // leaf_int_l (lvalue)
        expect(that % probe_int_lvalue.awaited);
        expect(that % probe_int_lvalue.resumed);
        expect(that % probe_int_lvalue.done);
        expect(that % !probe_int_lvalue.suspended);
        expect(that % !probe_int_lvalue.moved);
        expect(that % probe_int_lvalue.destroyed);

        // leaf_int_r (rvalue)
        expect(that % probe_int_rvalue.awaited);
        expect(that % probe_int_rvalue.resumed);
        expect(that % probe_int_rvalue.done);
        expect(that % !probe_int_rvalue.suspended);
        expect(that % !probe_int_rvalue.moved);
        expect(that % probe_int_rvalue.destroyed);

        // leaf_ptr
        expect(that % probe_ptr.awaited);
        expect(that % probe_ptr.done);
        expect(that % probe_ptr.resumed);
        expect(that % !probe_ptr.suspended);
        expect(that % !probe_ptr.moved);
        expect(that % probe_ptr.destroyed);

        // leaf_void
        expect(that % probe_void.awaited);
        expect(that % probe_void.done);
        expect(that % probe_void.resumed);
        expect(that % probe_void.suspended);
        expect(that % !probe_void.moved);
        expect(that % probe_void.destroyed);

        // leaf_throw
        expect(that % probe_throw.awaited);
        expect(that % probe_throw.done);
        expect(that % probe_throw.resumed);
        expect(that % probe_throw.suspended);
        expect(that % !probe_throw.moved);
        expect(that % probe_throw.destroyed);

        // nested
        expect(that % probe_nested.awaited);
        expect(that % probe_nested.done);
        expect(that % probe_nested.resumed);
        expect(that % probe_nested.suspended);
        expect(that % !probe_nested.moved);
        expect(that % !probe_nested.destroyed);
    };
}
