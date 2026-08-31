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

            constexpr std::int32_t inputL   = 42;
            constexpr std::int32_t inputR   = 58;
            constexpr std::int32_t inputPtr = 99;
            constexpr std::int32_t expected = inputL + inputR + inputPtr;

            std::vector<std::int32_t> trace;

            coroutine_probe probeIntLvalue;
            coroutine_probe probeIntRvalue;
            coroutine_probe probePtr;
            coroutine_probe probeVoid;
            coroutine_probe probeThrow;
            coroutine_probe probeNested;

            //NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            // These coroutine lambdas are invoked and completed synchronously by the test harness.
            // Their closure objects therefore outlive the coroutine execution.

            // -------------------------------
            // Leaf tasks
            // -------------------------------

            // int via ready_awaiter (lvalue)
            auto leafIntL = [&] -> test_task<std::int32_t> {
                trace.push_back(second);
                co_return run(as_task<std::int32_t>(dummies::ready_awaiter{inputL}));
            };

            // int via ready_awaiter (rvalue)
            auto leafIntR = [&] -> test_task<std::int32_t> {
                trace.push_back(third);
                co_return run(as_task<std::int32_t>(dummies::ready_awaiter{inputR}));
            };

            // unique_ptr via immediate_awaiter
            auto leafPtr = [&] -> test_task<std::unique_ptr<std::int32_t>> {
                trace.push_back(fourth);
                auto awaiter = dummies::immediate_awaiter{std::make_unique<std::int32_t>(inputPtr)};
                co_return run(as_task<std::unique_ptr<std::int32_t>>(std::move(awaiter)));
            };

            // void via immediate_awaiter
            auto leafVoid = [&] -> test_task<void> {
                trace.push_back(fifth);
                co_await as_task<void>(dummies::immediate_awaiter<void>{});
            };

            // throwing awaiter
            //NOLINTBEGIN(readability-convert-member-functions-to-static): Awaiter protocol.
            auto leafThrow = [&] -> test_task<void> {
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

                const std::int32_t valL = co_await leafIntL().set_probe(&probeIntLvalue);
                const std::int32_t valR = co_await std::move(leafIntR().set_probe(&probeIntRvalue));
                const auto ptr          = co_await leafPtr().set_probe(&probePtr);
                co_await leafVoid().set_probe(&probeVoid); // void task

                // Exception propagation check
                bool threw = false;
                try {
                    co_await leafThrow().set_probe(&probeThrow);
                } catch (const std::runtime_error&) {
                    threw = true;
                }
                expect(eq(threw, true));

                trace.push_back(seventh);
                co_return valL + valR + *ptr;
            };

            auto nested = make_nested();
            nested.set_probe(&probeNested);
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
            expect(eq(probeIntLvalue.awaited, true));
            expect(eq(probeIntLvalue.resumed, true));
            expect(eq(probeIntLvalue.done, true));
            expect(eq(probeIntLvalue.suspended, false));
            expect(eq(probeIntLvalue.moved, false));
            expect(eq(probeIntLvalue.destroyed, true));

            // leafIntR (rvalue)
            expect(eq(probeIntRvalue.awaited, true));
            expect(eq(probeIntRvalue.resumed, true));
            expect(eq(probeIntRvalue.done, true));
            expect(eq(probeIntRvalue.suspended, false));
            expect(eq(probeIntRvalue.moved, false));
            expect(eq(probeIntRvalue.destroyed, true));

            // leafPtr
            expect(eq(probePtr.awaited, true));
            expect(eq(probePtr.done, true));
            expect(eq(probePtr.resumed, true));
            expect(eq(probePtr.suspended, false));
            expect(eq(probePtr.moved, false));
            expect(eq(probePtr.destroyed, true));

            // leafVoid
            expect(eq(probeVoid.awaited, true));
            expect(eq(probeVoid.done, true));
            expect(eq(probeVoid.resumed, true));
            expect(eq(probeVoid.suspended, true));
            expect(eq(probeVoid.moved, false));
            expect(eq(probeVoid.destroyed, true));

            // leafThrow
            expect(eq(probeThrow.awaited, true));
            expect(eq(probeThrow.done, true));
            expect(eq(probeThrow.resumed, true));
            expect(eq(probeThrow.suspended, true));
            expect(eq(probeThrow.moved, false));
            expect(eq(probeThrow.destroyed, true));

            // nested
            expect(eq(probeNested.awaited, true));
            expect(eq(probeNested.done, true));
            expect(eq(probeNested.resumed, true));
            expect(eq(probeNested.suspended, true));
            expect(eq(probeNested.moved, false));
            expect(eq(probeNested.destroyed, false));
        };
    };
} //namespace

int main() {}
