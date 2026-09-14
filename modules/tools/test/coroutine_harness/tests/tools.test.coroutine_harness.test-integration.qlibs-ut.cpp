// SPDX-License-Identifier: Apache-2.0
// Integration tests for tools.test.coroutine_harness

import tools.test.coroutine_harness;

import ut;
import std;

using namespace ut;
using namespace tools::test::coroutine_harness;

namespace {
    //NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
    suite coroutine_harness_integration_tests = [] mutable {
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
                bool threw = false;
                try {
                    co_await leaf_throw().set_probe(&probe_throw);
                } catch (const std::runtime_error&) {
                    threw = true;
                }
                expect(eq(threw, true));

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
            std::vector<std::int32_t> expected_trace{first, second, third, fourth, fifth, sixth, seventh};
            // Since std::vector isn't output streamable, check element-wise equality.
            expect(eq(trace.size(), expected_trace.size()));
            for (std::size_t i = 0; i < trace.size() && i < expected_trace.size(); ++i) {
                //NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
                expect(eq(trace[i], expected_trace[i]));
            }

            // -------------------------------
            // Verify probes for all leaf tasks
            // -------------------------------

            // leafIntL (lvalue)
            expect(eq(probe_int_lvalue.awaited, true));
            expect(eq(probe_int_lvalue.resumed, true));
            expect(eq(probe_int_lvalue.done, true));
            expect(eq(probe_int_lvalue.suspended, false));
            expect(eq(probe_int_lvalue.moved, false));
            expect(eq(probe_int_lvalue.destroyed, true));

            // leafIntR (rvalue)
            expect(eq(probe_int_rvalue.awaited, true));
            expect(eq(probe_int_rvalue.resumed, true));
            expect(eq(probe_int_rvalue.done, true));
            expect(eq(probe_int_rvalue.suspended, false));
            expect(eq(probe_int_rvalue.moved, false));
            expect(eq(probe_int_rvalue.destroyed, true));

            // leafPtr
            expect(eq(probe_ptr.awaited, true));
            expect(eq(probe_ptr.done, true));
            expect(eq(probe_ptr.resumed, true));
            expect(eq(probe_ptr.suspended, false));
            expect(eq(probe_ptr.moved, false));
            expect(eq(probe_ptr.destroyed, true));

            // leafVoid
            expect(eq(probe_void.awaited, true));
            expect(eq(probe_void.done, true));
            expect(eq(probe_void.resumed, true));
            expect(eq(probe_void.suspended, true));
            expect(eq(probe_void.moved, false));
            expect(eq(probe_void.destroyed, true));

            // leafThrow
            expect(eq(probe_throw.awaited, true));
            expect(eq(probe_throw.done, true));
            expect(eq(probe_throw.resumed, true));
            expect(eq(probe_throw.suspended, true));
            expect(eq(probe_throw.moved, false));
            expect(eq(probe_throw.destroyed, true));

            // nested
            expect(eq(probe_nested.awaited, true));
            expect(eq(probe_nested.done, true));
            expect(eq(probe_nested.resumed, true));
            expect(eq(probe_nested.suspended, true));
            expect(eq(probe_nested.moved, false));
            expect(eq(probe_nested.destroyed, false));
        };
    };
} //namespace

int main() {}
