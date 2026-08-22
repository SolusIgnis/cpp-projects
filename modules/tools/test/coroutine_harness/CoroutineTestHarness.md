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
A `test_task` is a fully compatible **awaitable**. This means you can compose complex logic by having one coroutine `co_await` another. The harness handles the nested execution and tracking automatically.

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
import tools.test.coroutine_harness;

using namespace tools::test::coroutine_harness;

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

## 6. Test Dummies (`namespace dummies`)
When you need to test how a component (like a wrapper or a registry) interacts with the C++ coroutine protocol, you can use these "Trivial Awaiters" instead of writing full coroutines.

### `ready_awaiter<T>`
An awaiter that is **always ready**. It never suspends the calling coroutine.
* **Validation:** If it is accidentally suspended, it throws a `std::logic_error`.
* **Usage:** `co_await ready_awaiter<int>{42};`

### `immediate_awaiter<T>`
An awaiter that **suspends once** and then immediately resumes the caller.
* **Validation:** It uses *Symmetric Transfer* to resume the caller, making it a perfect tool for testing your code's suspension/resumption logic and stack safety.
* **Usage:** `co_await immediate_awaiter<void>{};`

### `adl::awaitable_by_adl<T>`
A dummy type that is made awaitable via a free-function `operator co_await` in its own namespace.
* **Validation:** Use this to verify that your wrappers correctly discover awaiters via Argument-Dependent Lookup (ADL) rather than just looking for member functions.

## 7. Helper: `as_task<T>(awaitable)`
This utility function template adapts any **awaitable** (like the test dummies) into a `test_task<T>`. This is extremely useful when you want to use the `run()` function on something that isn't a coroutine itself.

The adapter takes the awaitable **by value** and moves it into the `co_await` expression, so the value category of the argument is preserved at the call boundary: an **rvalue** argument is moved into the task, while an **lvalue** is copied. This ensures that the task owns its awaitable, so its lifetime is not dependent on the lifetime of the caller's object.

```cpp
using namespace tools::test::coroutine_harness;
using namespace tools::test::coroutine_harness::dummies;

void test_registry() {
    // 1. Create a simple 'Ready' awaiter
    auto awaiter = ready_awaiter<int>{42};

    // 2. Wrap it in a task so 'run()' can execute it
    auto task = as_task<int>(awaiter);

    // 3. Execute synchronously
    int result = run(task);

    assert(result == 42);
}
```
