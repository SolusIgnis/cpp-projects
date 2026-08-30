// SPDX-License-Identifier: Apache-2.0
// Unit tests for tools.test.coroutine_harness

/*
 * NOTE: These tests sit on the border between unit and integration tests.
 *       Due to the nature of coroutines and their promises and execution schedulers,
 *       `test_task`, `test_promise`, and `run` are tightly coupled to the point that
 *       the smallest possible unit to test is an integration of all 3. It is
 *       somewhere between impractical and impossible to bootstrap a testing
 *       environment that could test any of the 3 in isolation, so we bootstrap
 *       the testing of the behaviors of the set as a whole in order to develop a
 *       confidence in the correctness of all of the components.
 */

import tools.test.coroutine_harness;

import base.vocab;

import ut;
import std;

using namespace ut;
using namespace tools::test::coroutine_harness;

namespace {
    test_task<std::int32_t> echo(std::int32_t value)
    {
        co_return value;
    }

    test_task<test_task<std::int32_t>> make_echo(std::int32_t value, base::vocab::alias_ptr<coroutine_probe> probe = nullptr)
    {
        co_return echo(value).set_probe(probe);
    }

    //NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
    suite coroutine_harness_tests = [] mutable {
        "probe initialization"_test = [] mutable {
            const coroutine_probe probe;

            expect(eq(probe.done, false));
            expect(eq(probe.destroyed, false));
            expect(eq(probe.awaited, false));
            expect(eq(probe.suspended, false));
            expect(eq(probe.resumed, false));
            expect(eq(probe.moved, false));
            expect(eq(static_cast<std::int32_t>(probe.await_path), static_cast<std::int32_t>(coroutine_probe::path::none)));
        };

        "test_task default-constructs empty"_test = [] mutable {
            constexpr auto kiloword{1024};
            const test_task<std::int32_t> task1;
            const test_task<void> task2;
            const test_task<std::array<std::int32_t, kiloword>> task3;

            expect(eq(static_cast<bool>(task1), false));
            expect(eq(static_cast<bool>(task2), false));
            expect(eq(static_cast<bool>(task3), false));
        };

        "run returns value"_test = [] mutable {
            constexpr std::int32_t expected = 42;

            auto task = echo(expected);

            const auto result = run(task);

            expect(eq(result, expected));
        };

        "run returns void"_test = [] mutable {
            coroutine_probe probe;

            auto task = [] -> test_task<void> { co_return; }();

            task.set_probe(&probe);

            run(task);

            expect(eq(probe.awaited, true));
            expect(eq(probe.done, true));
        };

        "run throws on empty test_task"_test = [] mutable {
            test_task<void> task;

            bool threw = false;
            try {
                run(task);
            } catch (const std::logic_error&) {
                threw = true;
            }
            expect(eq(threw, true));
        };

        "operator co_await throws from empty test_task"_test = [] mutable {
            test_task<void> empty_task;

            bool threw = false;
            try {
                [[maybe_unused]] const auto awaiter = empty_task.operator co_await();
            } catch (const std::logic_error&) {
                threw = true;
            }
            expect(eq(threw, true));
        };

        "probe lifecycle"_test = [] mutable {
            coroutine_probe probe;

            {
                auto task = echo({});

                task.set_probe(&probe);

                [[maybe_unused]] const auto result = run(task);

                expect(eq(probe.awaited, true));
                expect(eq(probe.suspended, false));
                expect(eq(probe.resumed, true));
                expect(eq(probe.done, true));
                expect(eq(probe.destroyed, false));
                expect(eq(probe.moved, false));
                expect(
                    eq(static_cast<std::int32_t>(probe.await_path), static_cast<std::int32_t>(coroutine_probe::path::lvalue))
                );
            }

            expect(eq(probe.destroyed, true));
        };

        "rvalue await path"_test = [] mutable {
            coroutine_probe probe;

            auto task = echo({});
            task.set_probe(&probe);

            [[maybe_unused]] const auto result = run(std::move(task));

            expect(eq(probe.awaited, true));
            expect(eq(probe.resumed, true));
            expect(eq(probe.done, true));
            expect(eq(probe.moved, false)); //rvalue used in-place
            expect(eq(static_cast<std::int32_t>(probe.await_path), static_cast<std::int32_t>(coroutine_probe::path::rvalue)));
        };

        "premature destruction"_test = [] mutable {
            coroutine_probe probe;

            echo({}).set_probe(&probe); //temporary object destroyed at the ;

            expect(eq(probe.awaited, false));
            expect(eq(probe.done, false));
            expect(eq(probe.destroyed, true));
        };

        "premature destruction throws without probe"_test = [] mutable {
            bool threw = false;
            try {
                const auto unawaited_task = echo({});
            } catch (const std::logic_error&) {
                threw = true;
            }
            expect(eq(threw, true));
        };

        "swap exchanges tasks and preserves invariants"_test = [] mutable {
            constexpr std::int32_t expected1 = 1; //A
            constexpr std::int32_t expected2 = 2; //B

            coroutine_probe probe1{};
            coroutine_probe probe2{};
            {
                auto task1 = echo(expected1); //A
                auto task2 = echo(expected2); //B

                task1.set_probe(&probe1); //A
                task2.set_probe(&probe2); //B

                // Perform swap
                using std::swap;
                swap(task1, task2); //swap A and B

                // After swap, no lifecycle events should have happened yet
                expect(eq(probe1.destroyed, false));
                expect(eq(probe2.destroyed, false));
                expect(eq(probe1.awaited, false));
                expect(eq(probe2.awaited, false));
                expect(eq(probe1.moved, false));
                expect(eq(probe2.moved, false));

                const auto result1 = run(task1); // Run B

                // Probe behavior must follow the coroutine, not the wrapper
                expect(eq(probe1.awaited, false)); //A
                expect(eq(probe2.awaited, true));  //B

                const auto result2 = run(task2); // Run A

                // Probe behavior must follow the coroutine, not the wrapper
                expect(eq(probe1.awaited, true)); //A

                // Values must be swapped
                expect(eq(result1, expected2)); //B
                expect(eq(result2, expected1)); //A

                // Neither should be destroyed yet (still in scope)
                expect(eq(probe1.destroyed, false));
                expect(eq(probe2.destroyed, false));
            } // Destruction happens here
            expect(eq(probe1.destroyed, true));
            expect(eq(probe2.destroyed, true));
        };

        "swap is its own inverse operation (involution)"_test = [] mutable {
            constexpr std::int32_t expected1 = 1; //A
            constexpr std::int32_t expected2 = 2; //B

            coroutine_probe probe1{};
            coroutine_probe probe2{};

            auto task1 = echo(expected1); //A
            auto task2 = echo(expected2); //B

            task1.set_probe(&probe1); //A
            task2.set_probe(&probe2); //B

            // Perform double swap
            using std::swap;
            swap(task1, task2); //swap A and B
            swap(task1, task2); //swap B and A back

            const auto result1 = run(task1); // Run A

            // Probe behavior must follow the coroutine, not the wrapper
            expect(eq(probe1.awaited, true));  //A
            expect(eq(probe2.awaited, false)); //B

            const auto result2 = run(task2); // Run B

            // Probe behavior must follow the coroutine, not the wrapper
            expect(eq(probe2.awaited, true)); //B

            // Values must NOT be swapped
            expect(eq(result1, expected1)); //A
            expect(eq(result2, expected2)); //B
        };

        "self-swap is idempotent"_test = [] mutable {
            constexpr std::int32_t expected = 42;

            coroutine_probe probe{};

            {
                auto task = echo(expected);
                task.set_probe(&probe);

                // Perform self-swap
                using std::swap;
                swap(task, task);

                // After swap, no lifecycle events should have happened yet
                expect(eq(probe.destroyed, false));
                expect(eq(probe.awaited, false));
                expect(eq(probe.moved, false));

                // Still behaves normally
                const auto result = run(task);
                expect(eq(result, expected));

                expect(eq(probe.awaited, true));

                // Should not be destroyed yet (still in scope)
                expect(eq(probe.destroyed, false));
            } // Destruction happens here
            expect(eq(probe.destroyed, true));
        };

        "move assignment sets moved and destroys assigned-to"_test = [] mutable {
            constexpr std::int32_t expected  = 5;
            constexpr std::int32_t discarded = 10;

            coroutine_probe probe1;
            coroutine_probe probe2;

            auto task2 = echo(discarded);
            task2.set_probe(&probe2);
            {
                auto task1 = echo(expected);
                task1.set_probe(&probe1);

                task2 = std::move(task1); // move assignment
                expect(eq(probe1.moved, true));
                expect(eq(probe2.destroyed, false));
            } //destruction of discarded task occurs here when task1 destructor runs
            const auto result = run(task2);

            expect(eq(result, expected));
            expect(eq(probe1.moved, true));
            expect(eq(probe1.awaited, true));
            expect(eq(probe1.destroyed, false));
            expect(eq(probe2.moved, false));
            expect(eq(probe2.awaited, false));
            expect(eq(probe2.destroyed, true));
        };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-assign"
#pragma GCC diagnostic ignored "-Wself-move"
        "self assignment is safe"_test = [] mutable {
            constexpr std::int32_t expected = 42;
            coroutine_probe probe;

            auto task = echo(expected);
            task.set_probe(&probe);
            task = std::move(task);         // NOLINT(clang-diagnostic-self-move): testing safety of self-assignment
            expect(eq(probe.moved, false)); //self-assignment doesn't actually move
            expect(eq(run(task), expected));
        };
#pragma GCC diagnostic pop

        "task factory"_test = [] mutable {
            constexpr std::int32_t expected = 42;

            coroutine_probe factory_probe;
            coroutine_probe task_probe;

            test_task<std::int32_t> task;

            { //make sure factory is destroyed before we run the result task
                auto factory = make_echo(expected, &task_probe);
                factory.set_probe(&factory_probe);
                task = run(factory);

                expect(eq(factory_probe.done, true));
            }
            expect(eq(factory_probe.destroyed, true));

            expect(eq(task_probe.moved, true));
            expect(eq(task_probe.awaited, false));
            expect(eq(task_probe.destroyed, false));

            const auto result = run(task);

            expect(eq(result, expected));
            expect(eq(task_probe.awaited, true));
            expect(eq(task_probe.done, true));
        };

        "double await throws"_test = [] mutable {
            auto task = echo({});

            [[maybe_unused]] const auto result1 = run(task);

            bool threw = false;
            try {
                [[maybe_unused]] const auto result2 = run(task);
            } catch (const std::logic_error&) {
                threw = true;
            }
            expect(eq(threw, true));
        };

        "double await across swap throws"_test = [] mutable {
            auto task1 = echo({}); //A
            auto task2 = echo({}); //B

            [[maybe_unused]] const auto result1 = run(task1); //run A as task1

            using std::swap;
            swap(task1, task2); //swap A and B

            {
                bool threw = false;
                try {
                    [[maybe_unused]] const auto result2 = run(task2); //run A as task2
                } catch (const std::logic_error&) {
                    threw = true;
                }
                expect(eq(threw, true));
            }
            {
                bool threw = false;
                try {
                    [[maybe_unused]] const auto result3 = run(task1); //run B as task1
                } catch (const std::logic_error&) {
                    threw = true;
                }
                expect(eq(threw, false));
            }
        };

        "double await across move construction throws"_test = [] mutable {
            auto task1 = echo({});

            [[maybe_unused]] const auto result1 = run(task1);

            auto task2 = std::move(task1); //move construction

            expect(eq(static_cast<bool>(task1), false)); //NOLINT(bugprone-use-after-move): Testing moved-from state.

            bool threw = false;
            try {
                [[maybe_unused]] const auto result2 = run(task2);
            } catch (const std::logic_error&) {
                threw = true;
            }
            expect(eq(threw, true));
        };

        "double await across move assignment throws"_test = [] mutable {
            auto task1 = echo({});
            decltype(echo({})) task2;

            [[maybe_unused]] const auto result1 = run(task1);

            task2 = std::move(task1); //move assignment

            expect(eq(static_cast<bool>(task1), false)); //NOLINT(bugprone-use-after-move): Testing moved-from state.

            bool threw = false;
            try {
                [[maybe_unused]] const auto result2 = run(task2);
            } catch (const std::logic_error&) {
                threw = true;
            }
            expect(eq(threw, true));
        };

        "exception propagates"_test = [] mutable {
            auto task = [] -> test_task<std::int32_t> {
                throw std::runtime_error("boom");
                co_return {};
            }();

            bool threw = false;
            try {
                [[maybe_unused]] const auto result = run(task);
            } catch (const std::runtime_error&) {
                threw = true;
            }
            expect(eq(threw, true));
        };

        "stalled coroutine detected"_test = [] mutable {
            coroutine_probe probe;

            const auto make_task = [&] -> test_task<void> { co_await std::suspend_always{}; };

            auto task = make_task();

            task.set_probe(&probe);

            bool threw = false;
            try {
                run(task);
            } catch (const std::system_error& e) {
                if (e.code() == std::errc::resource_unavailable_try_again) {
                    threw = true;
                }
            }
            expect(eq(threw, true));
            expect(eq(probe.awaited, true));
        };

        "nested coroutine await"_test = [] mutable {
            constexpr std::int32_t dividend   = 42;
            constexpr std::int32_t divisor    = 7;
            constexpr std::int32_t subtrahend = 1;
            constexpr std::int32_t expected   = (dividend / divisor) - subtrahend;

            coroutine_probe probeA;
            coroutine_probe probeB;
            coroutine_probe probeC;
            coroutine_probe probeD;
            coroutine_probe probeE;

            // Lvalue task
            auto taskA = echo(dividend);
            taskA.set_probe(&probeA);

            // Temporary moved into taskB after probe is set
            auto taskB = echo(divisor).set_probe(&probeB);

            // Factory for rvalue task
            const auto make_taskC = [] -> test_task<std::int32_t> { co_return co_await echo(subtrahend); };

            const auto make_taskE = [&] -> test_task<std::int32_t> {
                std::int32_t quotient = 0; //42 / 7 == 6
                co_await [&] -> test_task<void> {
                    quotient = (co_await taskA) / (co_await taskB);
                    co_return;
                }()
                                    .set_probe(&probeD);

                const auto difference = quotient - co_await make_taskC().set_probe(&probeC); //6 - 1 == 5
                co_return difference;                                                        //5
            };
            auto taskE = make_taskE();
            taskE.set_probe(&probeE);

            const std::int32_t result = run(taskE);
            expect(eq(result, expected));

            // Assertions for taskA (unmoved lvalue)
            expect(eq(probeA.awaited, true));
            expect(eq(probeA.suspended, false));
            expect(eq(probeA.resumed, true));
            expect(eq(probeA.moved, false));
            expect(eq(probeA.done, true));
            expect(eq(probeA.destroyed, false));
            expect(eq(static_cast<std::int32_t>(probeA.await_path), static_cast<std::int32_t>(coroutine_probe::path::lvalue)));

            // Assertions for taskB (moved lvalue)
            expect(eq(probeB.awaited, true));
            expect(eq(probeB.suspended, false));
            expect(eq(probeB.resumed, true));
            expect(eq(probeB.moved, true));
            expect(eq(probeB.done, true));
            expect(eq(probeB.destroyed, false));
            expect(eq(static_cast<std::int32_t>(probeB.await_path), static_cast<std::int32_t>(coroutine_probe::path::lvalue)));

            // Assertions for taskC (rvalue)
            expect(eq(probeC.awaited, true));
            expect(eq(probeC.suspended, true));
            expect(eq(probeC.resumed, true));
            expect(eq(probeC.moved, false)); // rvalue temporary is never moved after probe is attached
            expect(eq(probeC.done, true));
            expect(eq(probeC.destroyed, true));
            expect(eq(static_cast<std::int32_t>(probeC.await_path), static_cast<std::int32_t>(coroutine_probe::path::rvalue)));

            // Assertions for taskD (unmaterialized rvalue)
            expect(eq(probeD.awaited, true));
            expect(eq(probeD.suspended, true));
            expect(eq(probeD.resumed, true));
            expect(eq(probeD.moved, false));
            expect(eq(probeD.done, true));
            expect(eq(probeD.destroyed, true));
            expect(eq(static_cast<std::int32_t>(probeD.await_path), static_cast<std::int32_t>(coroutine_probe::path::rvalue)));

            // Assertions for taskE (lvalue)
            expect(eq(probeE.awaited, true));
            expect(eq(probeE.suspended, true));
            expect(eq(probeE.resumed, true));
            expect(eq(probeE.moved, false)); // rvalue returned by lambda used in-place
            expect(eq(probeE.done, true));
            expect(eq(probeE.destroyed, false));
            expect(eq(static_cast<std::int32_t>(probeE.await_path), static_cast<std::int32_t>(coroutine_probe::path::lvalue)));
        };

        "continuation chaining preserves strict resume order"_test = [] mutable {
            std::vector<std::int32_t> trace;
            constexpr std::int32_t first  = 1;
            constexpr std::int32_t second = 2;
            constexpr std::int32_t third  = 3;
            constexpr std::int32_t fourth = 4;
            constexpr std::int32_t fifth  = 5;
            std::vector<std::int32_t> expected{first, second, third, fourth, fifth};

            const auto leaf = [&] -> test_task<void> {
                trace.push_back(third);
                co_return;
            };

            const auto mid = [&] -> test_task<void> {
                trace.push_back(second);
                co_await leaf();
                trace.push_back(fourth);
            };

            const auto root = [&] -> test_task<void> {
                trace.push_back(first);
                co_await mid();
                trace.push_back(fifth);
            };

            run(root());

            expect(eq(trace.size(), expected.size()));
            if (trace.size() == expected.size()) {
                for (std::size_t i = 0, size = trace.size(); i < size; ++i) {
                    expect(eq(trace.at(i), expected.at(i)));
                }
            }
        };
    };
} //namespace

int main() {}
