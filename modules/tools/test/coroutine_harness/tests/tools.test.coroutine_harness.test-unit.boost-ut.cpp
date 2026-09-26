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

import boost.ut;
import std;

using namespace boost::ext::ut;
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
} //namespace

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    "probe initialization"_test = [] mutable {
        const coroutine_probe probe;

        expect(that % !probe.done);
        expect(that % !probe.destroyed);
        expect(that % !probe.awaited);
        expect(that % !probe.suspended);
        expect(that % !probe.resumed);
        expect(that % !probe.moved);
        expect(eq(static_cast<std::int32_t>(probe.await_path), static_cast<std::int32_t>(coroutine_probe::path::none)));
    };

    "test_task default-constructs empty"_test = [] mutable {
        constexpr auto kiloword{1024};
        const test_task<std::int32_t> task1;
        const test_task<void> task2;
        const test_task<std::array<std::int32_t, kiloword>> task3;

        expect(that % !static_cast<bool>(task1));
        expect(that % !static_cast<bool>(task2));
        expect(that % !static_cast<bool>(task3));
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

        expect(that % probe.awaited);
        expect(that % probe.done);
    };

    "run throws on empty test_task"_test = [] mutable {
        test_task<void> task;

        expect(throws<std::logic_error>([&] { run(task); }));
    };

    "operator co_await throws from empty test_task"_test = [] mutable {
        test_task<void> empty_task;

        expect(throws<std::logic_error>([&] { [[maybe_unused]] const auto awaiter = empty_task.operator co_await(); }));
    };

    "probe lifecycle"_test = [] mutable {
        coroutine_probe probe;

        {
            auto task = echo({});

            task.set_probe(&probe);

            [[maybe_unused]] const auto result = run(task);

            expect(that % probe.awaited);
            expect(that % !probe.suspended);
            expect(that % probe.resumed);
            expect(that % probe.done);
            expect(that % !probe.destroyed);
            expect(that % !probe.moved);
            expect(eq(static_cast<std::int32_t>(probe.await_path), static_cast<std::int32_t>(coroutine_probe::path::lvalue)));
        }

        expect(that % probe.destroyed);
    };

    "rvalue await path"_test = [] mutable {
        coroutine_probe probe;

        auto task = echo({});
        task.set_probe(&probe);

        [[maybe_unused]] const auto result = run(std::move(task));

        expect(that % probe.awaited);
        expect(that % probe.resumed);
        expect(that % probe.done);
        expect(that % !probe.moved); //rvalue used in-place
        expect(eq(static_cast<std::int32_t>(probe.await_path), static_cast<std::int32_t>(coroutine_probe::path::rvalue)));
    };

    "premature destruction"_test = [] mutable {
        coroutine_probe probe;

        echo({}).set_probe(&probe); //temporary object destroyed at the ;

        expect(that % !probe.awaited);
        expect(that % !probe.done);
        expect(that % probe.destroyed);
    };

    "premature destruction throws without probe"_test = [] mutable {
        expect(throws<std::logic_error>([&] { const auto unawaited_task = echo({}); }));
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
            expect(that % !probe1.destroyed);
            expect(that % !probe2.destroyed);
            expect(that % !probe1.awaited);
            expect(that % !probe2.awaited);
            expect(that % !probe1.moved);
            expect(that % !probe2.moved);

            const auto result1 = run(task1); // Run B

            // Probe behavior must follow the coroutine, not the wrapper
            expect(that % !probe1.awaited); //A
            expect(that % probe2.awaited);  //B

            const auto result2 = run(task2); // Run A

            // Probe behavior must follow the coroutine, not the wrapper
            expect(that % probe1.awaited); //A

            // Values must be swapped
            expect(eq(result1, expected2)); //B
            expect(eq(result2, expected1)); //A

            // Neither should be destroyed yet (still in scope)
            expect(that % !probe1.destroyed);
            expect(that % !probe2.destroyed);
        } // Destruction happens here
        expect(that % probe1.destroyed);
        expect(that % probe2.destroyed);
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
        expect(that % probe1.awaited);  //A
        expect(that % !probe2.awaited); //B

        const auto result2 = run(task2); // Run B

        // Probe behavior must follow the coroutine, not the wrapper
        expect(that % probe2.awaited); //B

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
            expect(that % !probe.destroyed);
            expect(that % !probe.awaited);
            expect(that % !probe.moved);

            // Still behaves normally
            const auto result = run(task);
            expect(eq(result, expected));

            expect(that % probe.awaited);

            // Should not be destroyed yet (still in scope)
            expect(that % !probe.destroyed);
        } // Destruction happens here
        expect(that % probe.destroyed);
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
            expect(that % probe1.moved);
            expect(that % !probe2.destroyed);
        } //destruction of discarded task occurs here when task1 destructor runs
        const auto result = run(task2);

        expect(eq(result, expected));
        expect(that % probe1.moved);
        expect(that % probe1.awaited);
        expect(that % !probe1.destroyed);
        expect(that % !probe2.moved);
        expect(that % !probe2.awaited);
        expect(that % probe2.destroyed);
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-assign"
#pragma GCC diagnostic ignored "-Wself-move"
    "self assignment is safe"_test = [] mutable {
        constexpr std::int32_t expected = 42;
        coroutine_probe probe;

        auto task = echo(expected);
        task.set_probe(&probe);
        task = std::move(task);      // NOLINT(clang-diagnostic-self-move): testing safety of self-assignment
        expect(that % !probe.moved); //self-assignment doesn't actually move
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

            expect(that % factory_probe.done);
        }
        expect(that % factory_probe.destroyed);

        expect(that % task_probe.moved);
        expect(that % !task_probe.awaited);
        expect(that % !task_probe.destroyed);

        const auto result = run(task);

        expect(eq(result, expected));
        expect(that % task_probe.awaited);
        expect(that % task_probe.done);
    };

    "double await throws"_test = [] mutable {
        auto task = echo({});

        [[maybe_unused]] const auto result1 = run(task);

        expect(throws<std::logic_error>([&] { [[maybe_unused]] const auto result2 = run(task); }));
    };

    "double await across swap throws"_test = [] mutable {
        auto task1 = echo({}); //A
        auto task2 = echo({}); //B

        [[maybe_unused]] const auto result1 = run(task1); //run A first as task1

        using std::swap;
        swap(task1, task2); //swap A and B

        //run A again as task2 (while task2 has never yet run)
        expect(throws<std::logic_error>([&] { [[maybe_unused]] const auto result2 = run(task2); }));

        //run B once as task1 (even though task1 was run as A previously)
        expect(nothrow([&] { [[maybe_unused]] const auto result3 = run(task1); }));
    };

    "double await across move construction throws"_test = [] mutable {
        auto task1 = echo({});

        [[maybe_unused]] const auto result1 = run(task1);

        auto task2 = std::move(task1); //move construction

        expect(eq(static_cast<bool>(task1), false)); //NOLINT(bugprone-use-after-move): Testing moved-from state.

        expect(throws<std::logic_error>([&] { [[maybe_unused]] const auto result2 = run(task2); }));
    };

    "double await across move assignment throws"_test = [] mutable {
        auto task1 = echo({});
        decltype(echo({})) task2;

        [[maybe_unused]] const auto result1 = run(task1);

        task2 = std::move(task1); //move assignment

        expect(eq(static_cast<bool>(task1), false)); //NOLINT(bugprone-use-after-move): Testing moved-from state.

        expect(throws<std::logic_error>([&] { [[maybe_unused]] const auto result2 = run(task2); }));
    };

    "exception propagates"_test = [] mutable {
        auto task = [] -> test_task<std::int32_t> {
            throw std::runtime_error("boom");
            co_return {};
        }();

        expect(throws<std::runtime_error>([&] { [[maybe_unused]] const auto result = run(task); }));
    };

    "stalled coroutine detected"_test = [] mutable {
        coroutine_probe probe;

        const auto make_task = [&] -> test_task<void> { co_await std::suspend_always{}; };

        auto task = make_task();

        task.set_probe(&probe);

        bool threw           = false;
        bool wrong_errc      = false;
        bool wrong_exception = false;

        try {
            run(task);
        } catch (const std::system_error& e) {
            if (e.code() == std::errc::resource_unavailable_try_again) {
                threw = true;
            } else {
                wrong_errc = true;
            }
        } catch (...) {
            wrong_exception = true;
        }
        expect(that % threw);
        expect(that % !wrong_errc);
        expect(that % !wrong_exception);

        expect(that % probe.awaited);
    };

    "nested coroutine await"_test = [] mutable {
        constexpr std::int32_t dividend   = 42;
        constexpr std::int32_t divisor    = 7;
        constexpr std::int32_t subtrahend = 1;
        constexpr std::int32_t expected   = (dividend / divisor) - subtrahend;

        coroutine_probe probe_a;
        coroutine_probe probe_b;
        coroutine_probe probe_c;
        coroutine_probe probe_d;
        coroutine_probe probe_e;

        // Lvalue task
        auto task_a = echo(dividend);
        task_a.set_probe(&probe_a);

        // Temporary moved into taskB after probe is set
        auto task_b = echo(divisor).set_probe(&probe_b);

        // Factory for rvalue task
        const auto make_task_c = [] -> test_task<std::int32_t> { co_return co_await echo(subtrahend); };

        //NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
        // This coroutine lambda is invoked and completed synchronously by the test harness.
        // Its closure object therefore outlives the coroutine execution.
        const auto make_task_e = [&] -> test_task<std::int32_t> {
            std::int32_t quotient = 0; //42 / 7 == 6
            co_await [&] -> test_task<void> {
                quotient = (co_await task_a) / (co_await task_b);
                co_return;
            }()
                                .set_probe(&probe_d);

            const auto difference = quotient - co_await make_task_c().set_probe(&probe_c); //6 - 1 == 5
            co_return difference;                                                          //5
        };
        auto task_e = make_task_e();
        task_e.set_probe(&probe_e);
        //NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)

        const std::int32_t result = run(task_e);
        expect(eq(result, expected));

        // Assertions for taskA (unmoved lvalue)
        expect(that % probe_a.awaited);
        expect(that % !probe_a.suspended);
        expect(that % probe_a.resumed);
        expect(that % !probe_a.moved);
        expect(that % probe_a.done);
        expect(that % !probe_a.destroyed);
        expect(eq(static_cast<std::int32_t>(probe_a.await_path), static_cast<std::int32_t>(coroutine_probe::path::lvalue)));

        // Assertions for taskB (moved lvalue)
        expect(that % probe_b.awaited);
        expect(that % !probe_b.suspended);
        expect(that % probe_b.resumed);
        expect(that % probe_b.moved);
        expect(that % probe_b.done);
        expect(that % !probe_b.destroyed);
        expect(eq(static_cast<std::int32_t>(probe_b.await_path), static_cast<std::int32_t>(coroutine_probe::path::lvalue)));

        // Assertions for taskC (rvalue)
        expect(that % probe_c.awaited);
        expect(that % probe_c.suspended);
        expect(that % probe_c.resumed);
        expect(that % !probe_c.moved); // rvalue temporary is never moved after probe is attached
        expect(that % probe_c.done);
        expect(that % probe_c.destroyed);
        expect(eq(static_cast<std::int32_t>(probe_c.await_path), static_cast<std::int32_t>(coroutine_probe::path::rvalue)));

        // Assertions for taskD (unmaterialized rvalue)
        expect(that % probe_d.awaited);
        expect(that % probe_d.suspended);
        expect(that % probe_d.resumed);
        expect(that % !probe_d.moved);
        expect(that % probe_d.done);
        expect(that % probe_d.destroyed);
        expect(eq(static_cast<std::int32_t>(probe_d.await_path), static_cast<std::int32_t>(coroutine_probe::path::rvalue)));

        // Assertions for taskE (lvalue)
        expect(that % probe_e.awaited);
        expect(that % probe_e.suspended);
        expect(that % probe_e.resumed);
        expect(that % !probe_e.moved); // rvalue returned by lambda used in-place
        expect(that % probe_e.done);
        expect(that % !probe_e.destroyed);
        expect(eq(static_cast<std::int32_t>(probe_e.await_path), static_cast<std::int32_t>(coroutine_probe::path::lvalue)));
    };

    "continuation chaining preserves strict resume order"_test = [] mutable {
        std::vector<std::int32_t> trace;
        constexpr std::int32_t first  = 1;
        constexpr std::int32_t second = 2;
        constexpr std::int32_t third  = 3;
        constexpr std::int32_t fourth = 4;
        constexpr std::int32_t fifth  = 5;
        const std::vector<std::int32_t> expected{first, second, third, fourth, fifth};

        //NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
        // This coroutine lambda is invoked and completed synchronously by the test harness.
        // Its closure object therefore outlives the coroutine execution.
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
        //NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)

        run(root());

        expect(that % trace == expected);
    };
}
