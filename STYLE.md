# cpp-projects Coding Style Guide

This document defines the **authoritative coding style** for the `net.telnet` project.

The style is intentionally strict and explicit. It exists to support:

* Protocol-heavy, FSM-oriented code
* C++23+ modules, concepts, and constraints
* Long-term maintainability over short-term convenience
* Minimal diff churn and maximum review clarity

This document explains **why** the rules exist. The **exact formatting rules** are enforced by `.clang-format` and are considered normative.

---

## 1. Authority and Scope

* `.clang-format` is **normative**.
* This document is **explanatory**.
* Any code in this repository **must** conform to the formatter.
* Manual formatting that contradicts the formatter is not allowed.

If the formatter produces undesirable output, the formatter configuration—not the code—must be discussed and updated.

---

## 2. Core Philosophy

### 2.1 Human Intent Over Formatter Cleverness

The formatter is deliberately configured to avoid "clever" rewrites:

* Expressions are not aggressively reflowed
* Binary operations are not arbitrarily split
* Manual line breaks are respected

**Rationale:**
FSM logic, protocol parsing, and negotiation state machines are easier to understand when the original logical grouping is preserved.

---

### 2.2 Readability Beats Compactness

We favor vertical clarity over horizontal density:

* No one-line `if` statements
* No compressed `case:` bodies
* Clear visual separation between logical blocks

**Rationale:**
Protocol code is read far more often than it is written. Compact code saves keystrokes; readable code saves time.

---

### 2.3 Stability Over Fashion

This project optimizes for:

* Stable diffs
* Predictable formatting
* Minimal reformatting churn

Alignment rules are intentionally conservative, and many auto-alignment features are disabled.

---

## 3. Indentation and Structure

### 3.1 Indentation

* Indentation width: **4 spaces**
* Tabs are never used
* Continuation indentation is explicit and consistent

Namespaces are always indented, and nested namespaces are not compacted.

**Rationale:**
Protocol code frequently nests states, transitions, and handlers. Consistent indentation prevents visual ambiguity.

---

### 3.2 Braces

* Braces follow the LLVM-style "attach" convention
* Control statements do not introduce extra line breaks
* Empty blocks are preserved intentionally

**Rationale:**
FSM transitions often use empty or placeholder blocks to express intent. These must remain visible.

---

## 4. Control Flow Rules (FSM-Oriented)

### 4.1 Conditionals

* `if` statements are never allowed on a single line
* Ternary operators are broken for clarity

```cpp
if (state == fsm_state::negotiating
    && option.supports_remote()) {
    // ...
}
```

**Rationale:**
FSM transitions frequently encode protocol invariants. One-line conditionals obscure those invariants.

---

### 4.2 Switch Statements

* `case` labels are never collapsed
* Short case statements may be aligned for readability

```cpp
case command::will:
    handle_will();
    break;
```

---

### 4.3 Lambdas

Short lambdas are allowed on a single line:

```cpp
auto accepts = [](option::id_num) noexcept { return true; };
```

**Rationale:**
Predicates and callbacks are first-class constructs in this codebase.

---

## 5. Alignment Rules

### 5.1 What Is Aligned

* Consecutive assignments
* Short `case` statements
* Trailing comments

```cpp
binary        = 0x00, // RFC 856
suppress_ga  = 0x03, // RFC 858
```

### 5.2 What Is *Not* Aligned

* Function declarations
* Macros
* Declarations across comments or empty lines

**Rationale:**
Over-alignment increases diff noise and reduces merge stability.

---

## 6. Comments and Documentation

### 6.1 Documentation Style

* Doxygen-style comments are used throughout
* Formatter will **not reflow** documentation text
* Manual line breaks are preserved

**Rationale:**
Documentation frequently references RFC language and must remain stable.

---

### 6.2 Pragmas

Comments starting with `@` are treated as semantic markers and are never reformatted.

---

## 7. Modern C++ and Modules

This project assumes a **modern C++ baseline**:

* C++23 or later
* Modules are preferred over headers
* Concepts and `requires` clauses are formatted for clarity

```cpp
template<typename T>
    requires Negotiable<T>
void negotiate(T&& value);
```

**Rationale:**
Constraints are part of the API and must be visually distinct.

---

## 8. Includes and Imports

* Include order is preserved
* Imports are not auto-sorted
* Grouping is intentional and semantic

**Rationale:**
Module boundaries and import order often convey architectural intent.

---

## 9. Tooling and Enforcement

* Formatting is enforced via `clang-format`
* CI runs formatting in **check-only** mode
* Developers are expected to format locally

No automatic formatting is applied during CI.

### 9.1 Fixed-Layout Enumerations / Tables

In rare cases, we define enumerations or other constructs where **column alignment is critical** for readability. For example, protocol registries, opcode tables, or Telnet option enumerations are formatted like a table with three aligned columns:

* **Enumerator name**
* **Value / code**
* **Description / reference comment**

### Guidelines

1. **Disable clang-format inside the table**
   Use the following pattern to scope formatting exceptions:

```cpp
enum class example::code_num : std::uint8_t {
// clang-format off: preserve column alignment
/* ====================================================================== *
 *  Name           Code         Description                               *
 * ====================================================================== */
    foo           = 0x01, ///< Foo description (@see Foo reference)
    bar           = 0x02, ///< Bar description (@see Bar reference)
    baz           = 0x03  ///< Baz description (@see Baz reference)
// clang-format on
};
```

* Only the formatting of the enumeration body is disabled; the `enum class` declaration and braces are still formatted normally.
* Include the short comment "preserve column alignment" immediately after the `clang-format off` directive as in the example:

```cpp
// clang-format off: preserve column alignment
```

2. **Document the reason**
   In the Doxygen block immediately above the enum, use a @remark to explain why formatting is disabled:

```cpp
/**
 * @brief Enumeration of Example Codes.
 *
 * @remark The formatting of this protocol registry enumeration is
 * intentionally fixed to preserve column alignment of names, codes,
 * and reference comments. clang-format is disabled within the
 * braces of the enumeration.
 */
```

3. **Maintain alignment manually**

   * Keep enumerators, =, values, and trailing comments visually aligned.
   * Update ASCII table headers if new items are added that change horizontal spacing.
   * Ensure all new entries follow the existing column spacing (or adjust to accommodate longer entries).

4. **Keep it rare**
   This pattern should be used **only when alignment is critical for readability**. Most code should adhere to the project’s clang-format rules.


---

## 10. Changing the Style

Changes to formatting rules:

* Must be discussed
* Must update `.clang-format`
* Must update this document

Style changes are **versioned decisions**, not casual preferences.

---

## 11. Summary

This style guide exists to make protocol code:

* Easier to read
* Easier to review
* Easier to maintain

If a rule feels restrictive, it is probably doing its job.
