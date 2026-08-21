// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file tools.test.coroutine_harness-runner.cppm
 * @version 0.3.0
 * @date April 12, 2026
 *
 * @copyright © 2026 Jeremy Murphy and any Contributors
 * @par License: @parblock
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. @endparblock
 *
 * @brief Synchronous coroutine execution utilities.
 *
 * @details
 * Defines:
 * - `run()`               = executes a coroutine to completion
 * - `test_runner_entry()` = coroutine entry trampoline
 * - `as_task()`           = wraps awaitables as `test_task`s
 *
 * Executes coroutine synchronously
 * Propagates exceptions
 * Detects stalled coroutines
 *
 * Depends only on `:task`
 */

//Module partition interface unit
export module tools.test.coroutine_harness:runner;

import std; //NOLINT

import :task; ///< @see "tools.test.coroutine_harness-task.cppm"

export namespace tools::test::coroutine_harness {
    ///@brief Wrap a trivial awaiter or non-coroutine awaitable in a `test_task` so that it can be run as a task.
    template<typename T, typename Awaitable>
    [[nodiscard]] auto as_task(Awaitable&& awaitable) -> test_task<T>
    {
        co_return co_await std::forward<Awaitable>(awaitable);
    }

    /// @brief Entry trampoline used by `run`.
    template<typename Task>
    auto test_runner_entry(Task&& task) // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
        -> std::remove_reference_t<Task>
    {
        co_return co_await std::forward<Task>(task);
    }

    /**
     * @brief Execute a coroutine synchronously.
     *
     * @throws std::system_error if coroutine stalls
     * @throws any exception thrown inside the coroutine
     */
    template<typename Task>
    [[nodiscard]] decltype(auto) run(Task&& task)
    {
        auto entry_point{test_runner_entry(std::forward<Task>(task))};

        auto awaiter{std::forward<Task>(entry_point).operator co_await()};

        if (!awaiter.await_ready()) {
            (awaiter.await_suspend(std::noop_coroutine())).resume();
        }

        if (awaiter.my_handle.done()) {
            return awaiter.await_resume();
        } else {
            throw std::system_error(
                std::make_error_code(std::errc::resource_unavailable_try_again),
                "Coroutine failed to complete (stalled at suspension point)"
            );
        }
    }
} //namespace tools::test::coroutine_harness
