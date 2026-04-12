// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @module tools.test.coroutine_harness
 * @file tools.test.coroutine_harness.cppm
 * @version 0.2.0
 * @date March 30, 2026
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
 * @brief Coroutine test harness with synchronous runner.
 *
 * This module provides `test_task` and `test_promise` types for unit-testing coroutines
 * in a synchronous, deterministic fashion. It tracks coroutine lifecycle events via
 * `coroutine_probe` and ensures safe destruction, double-await detection, and exception propagation.
 *
 * @remark Detects double-awaits and moves.
 * @remark Tracks suspension, resumption, and destruction via `coroutine_probe`.
 * @remark Supports `void` and non-`void` coroutine return types.
 * @remark Provides a synchronous runner `run()` that executes coroutines to completion or throws if the coroutine stalls or propagates exceptions.
 *
 * @example @parblock
 * ## Example Usage
 * @code ```cpp
 * using namespace tools::test;
 *
 * coroutine_probe probe;
 *
 * // Create a test coroutine task that returns 42
 * auto my_task = []() -> test_task<int>
 * {
 *     co_return 42;
 * }();
 *
 * my_task.set_probe(&probe);
 *
 * // Run and await the coroutine within this scope
 * {
 *     int result = run(my_task);
 *
 *     using enum coroutine_probe::path;
 *     assert(result == 42);
 *     assert(probe.awaited);    // co_await was called
 *     assert(probe.resumed);    // coroutine resumed after suspension
 *     assert(!probe.moved);     // task was not moved (lvalue reference passed to run)
 *     assert(probe.done);       // final_suspend reached
 *     assert(!probe.destroyed); // coroutine frame not destroyed yet
 *     assert(probe.await_path == lvalue); // lvalue co_await was called
 * }
 *
 * // After leaving the scope, the coroutine frame is destroyed
 * assert(probe.destroyed);
 * ``` @endcode @endparblock
 *
 * @remark `operator co_await()` distinguishes lvalue vs rvalue `test_task` to track await paths.
 * @remark `test_runner_entry` and `run()` ensure the coroutine completes synchronously, with any exceptions propagated.
 */

//Primary module interface unit
export module tools.test.coroutine_harness;

//Export all partition interfaces
export import :test_task; ///< @see "tools.test.coroutine_harness-test_task.cppm"
