<!-- SPDX-License-Identifier: Apache-2.0 -->
<!-- SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors -->
#  Testing Framework Guide: `qlibs/ut`

To test our code, we use **`ut`** (from qlibs). It is a modern, macro-free C++20 framework. Unlike older frameworks (like GTest), `ut` is designed to be as close to standard C++ as possible, utilizing lambdas and custom string literals.

---

## 1. The `mutable` Requirement (Critical)

In `ut`, the behavior of a test depends on the lambda signature. You will notice almost all our tests are declared with `[] mutable`.

* **Runtime Execution:** Adding `mutable` (e.g., `"name"_test = [] mutable { ... };`) tells the framework to execute the test at **runtime**.
* **Compile-time Execution:** If you omit `mutable`, the framework attempts to run the test inside a `static_assert`. Since our coroutine harness involves dynamic memory and complex handles, your tests will usually fail to compile if you forget `mutable`.

---

## 2. Framework Syntax

### The `suite`
Tests are grouped into a `suite`. This is a global or namespace-level object that keeps our test output organized.
```cpp
suite coroutine_harness_tests = [] mutable {
    // All individual tests go here
};
```

### The `_test` Literal
Individual test cases are defined by adding the `_test` suffix to a string literal.
```cpp
"description of the test"_test = [] mutable {
    // Test logic
};
```

### Expectations (`expect` and `eq`)
The framework does **not** allow passing a raw `bool` directly to the `expect` function (e.g., `expect(my_bool)` is invalid). It requires a comparison object to provide better error diagnostics.

* **Equality checks:** `expect(eq(result, 42));`
* **Boolean checks:** To check if something is true or false, you must compare it against the literal:
    * `expect(eq(probe.done, true));`
    * `expect(eq(is_valid, false));`

---

## 3. Common Testing Patterns

### Scope-Based Lifecycle Testing
Since `ut` tests are just lambdas, we use standard C++ block scopes `{ }` inside a test to trigger destructors. This is essential for verifying that coroutine frames are cleaned up correctly.

```cpp
"lifecycle_test"_test = [] mutable {
    coroutine_probe probe;
    {
        auto task = some_coro();
        task.set_probe(&probe);
        // Task is alive here
    } 
    // Task went out of scope and was destroyed
    expect(eq(probe.destroyed, true));
};
```

### Handling Exceptions
When testing that code correctly throws, use a standard try-catch block and a boolean flag.
```cpp
"error_propagation_test"_test = [] mutable {
    bool threw = false;
    try {
        run(faulty_task());
    } catch (const std::runtime_error&) {
        threw = true;
    }
    expect(eq(threw, true));
};
```

## 4. Summary/Reference

| Requirement           | Rule                                               |
| :-------------------- | :------------------------------------------------- |
| **Runtime Execution** | Always use **`[] mutable`** for your test lambdas. |
| **Defining Tests**    | Use the string literal syntax: `"test name"_test`. |
| **Assertion Style**   | Always use **`expect(eq(actual, expected))`**.     |
| **Boolean Checks**    | Explicitly compare to `true` or `false`.           |
