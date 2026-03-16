<!-- SPDX-License-Identifier: Apache-2.0 -->
<!-- SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors -->
# 🛠️ Coroutine Test Harness API Guide

To test our asynchronous components, we use a specialized **Coroutine Test Harness**. This tool allows you to write unit tests for C++ coroutines that behave like regular, synchronous functions—making your tests deterministic and easy to debug.

---

## 1. Core Components

### `test_task<T>`
This is the **Return Type** for your test coroutines. It acts as a handle to the coroutine's state.
* If a function returns an integer asynchronously, use `test_task<int>`.
* If it returns nothing, use `test_task<void>`.

### `run(task)`
This is the primary execution function. It takes a `test_task` and forces it to run to completion immediately on the current thread. 
* **Returns:** The value produced by `co_return` (or `void`).
* **Exceptions:** If the coroutine throws, `run()` re-throws it into your test so you can catch it with your test framework.
* **Stalls:** If the coroutine pauses and never resumes (a "stall"), `run()` throws a `system_error` rather than hanging your test suite.

---

## 2. Using the `coroutine_probe`
The `coroutine_probe` is a "spy" object. You attach it to a task to verify its internal lifecycle.



### Probe Flags
After calling `run()`, you can assert against these boolean flags:

| Flag          | Description                                      |
| :------------ | :----------------------------------------------- |
| `.awaited`    | Was `co_await` (or `run`) called on this task?   |
| `.suspended`  | Did the coroutine pause at least once?           |
| `.resumed`    | Did the coroutine wake up after a suspension?    |
| `.done`       | Did the coroutine reach its final `co_return`?   |
| `.destroyed`  | Was the coroutine frame cleaned up?              |
| `.moved`      | Was the task object moved to another variable?   |
| `.await_path` | Records if it was an `lvalue` or `rvalue` await. |

---

## 3. Key Feature: `test_task` is Awaitable
A `test_task` is a fully compatible **Awaitable**. This means you can compose complex logic by having one coroutine `co_await` another. The harness handles the nested execution and tracking automatically.

```cpp
auto sub_task() -> test_task<int> {
    co_return 10;
}

auto main_task() -> test_task<int> {
    // You can await test_tasks inside other test_tasks!
    int value = co_await sub_task(); 
    co_return value + 32;
}
```

## 4. Quick Start Example
```cpp
import net.telnet.test_support;

using namespace net::telnet::test_support::coroutine_harness;

void test_example() {
    // 1. Setup the spy
    coroutine_probe probe;

    // 2. Create the coroutine
    auto my_coro = []() -> test_task<int> {
        co_return 42;
    }();

    // 3. Attach the probe (supports chaining)
    my_coro.set_probe(&probe);

    // 4. Run to completion synchronously
    int result = run(my_coro);

    // 5. Assert expectations
    assert(result == 42);
    assert(probe.done == true);
    assert(probe.awaited == true);
}
```

## 5. Safety Guardrails
​The harness is "strict" to help you catch common async bugs early:
* **​Double Await:** Throws `logic_error` if you try to await the same task twice.
​* **Dangling Frames:** Throws `logic_error` if a coroutine is destroyed before finishing without a probe attached.
​* **Strict Moves:** Tracks if a task was moved, helping you verify that your async factory functions are behaving correctly.
