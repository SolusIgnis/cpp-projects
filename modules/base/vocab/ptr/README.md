# Pointer Vocabulary Module

The `base.vocab.ptr` module provides a collection of lightweight, non-owning pointer types that make object relationships explicit in the type system.

Traditional C++ offers only two fundamental mechanisms for referring to existing objects:

- references
- raw pointers

While both are efficient, neither fully communicates intent.

A reference implies a required relationship but cannot be reseated and is not itself an object. A raw pointer is a reseatable object but conflates many different meanings: ownership, optionality, object observation, iterator traversal, array indexing, and more.

As a result, APIs frequently communicate important semantic requirements only through documentation, naming conventions, runtime assertions, or developer discipline.

The vocabulary pointer library addresses this problem by treating common pointer semantics as distinct types rather than informal conventions.

For example:

```cpp
dependency_ptr<logger> logger_;
required_ptr<widget> selected_widget_;
alias_ptr<widget> hovered_widget_;
cursor_ptr<char> current_;
iterator_ptr<widget> iter;
```

Although each object ultimately stores only an address, the type itself communicates:

- whether the relationship may be empty
- whether binding must occur through references or pointers
- which traversal semantics are permitted
- which operations are intentionally unavailable

This shifts many categories of misuse from runtime errors and documentation violations into compile-time diagnostics.

To complement these vocabulary types, the module also provides the free function `pointer_to`, a generic pointer factory that produces pointer objects from references through `std::pointer_traits`, allowing vocabulary pointers and other conforming pointer types to be formed using a single, declarative interface:

```cpp
auto ptr = pointer_to<required_ptr>(object);
```

---
## Design Philosophy
---

The library adopts three guiding principles to realize these goals.

### Pointers Are First-Class Types

Vocabulary pointers are not wrappers around "real" pointers.

A vocabulary pointer is itself a pointer type with its own semantics, invariants, comparisons, formatting behavior, and standard-library integrations.

Internally, each vocabulary pointer stores an address and exposes pointer operations, but the type system is used to encode intent that raw pointers alone cannot express.

### Vocabulary Pointers Are Non-Owning

To preserve the simplicity, efficiency, and familiarity of native operations, the vocabulary pointers are designed strictly as **vocabulary pointers**, not ownership abstractions.

They:

- do **not** own objects
- do **not** participate in lifetime management
- do **not** perform reference counting
- do **not** allocate memory
- store exactly one address
- preserve native pointer-identity semantics
- express and enforce clear semantic contracts

### Semantics Are Policies

Rather than implementing each vocabulary pointer independently, the library decomposes pointer behavior into a small collection of orthogonal policy categories:

- traversal
- reference binding
- pointer binding
- nullability

Concrete pointer types are created by selecting exactly one policy from each category.

This allows the library to express distinct semantic contracts while sharing a common implementation foundation.

The result is a family of pointer types that are:

- lightweight
- zero-overhead
- policy-driven
- structurally consistent
- semantically explicit

This architecture provides several benefits:

- Semantic intent becomes visible at the type level.
- Invalid pointer states become unrepresentable.
- Common misuse patterns are diagnosed at compile time.

The remainder of this document describes the concrete vocabulary pointer types, the policy system used to define them, and the implementation framework that realizes their behavior.

---
## Concrete Pointer Types
---

The module provides five concrete vocabulary pointer types. Each represents a distinct semantic contract with a uniform storage representation.

All five types are:

- non-owning
- single-address representations
- pointer-identity preserving
- rebindable
- lightweight value types
- compatible with standard hashing and formatting facilities

They differ only in the operations and invariants selected through the policy framework.

### Required Dependency Alias (`dependency_ptr`)
`base.vocab.ptr:dependency_ptr`

A compile-time validated, reference-bound, required-dependency alias pointer.

```cpp
widget w;

dependency_ptr<widget> dep_ptr{w};
```

Characteristics:

- must always contain a valid address
- can not be default constructed
- can not be assigned `nullptr`
- can not be constructed from raw pointers
- can not be constructed from pointer-like types
- supports rebinding through references
- does not support pointer arithmetic

`dependency_ptr` models required object dependencies where an object is expected to exist and reference binding is enforced over address binding.

Typical use cases include:

- constructor-injected dependencies
- service references
- required collaborators
- long-lived object relationships

---

### Required-Object Alias (`required_ptr`)
`base.vocab.ptr:required_ptr`

A general-purpose, required-object alias pointer.

```cpp
widget object;

required_ptr<widget> ptr{object};
required_ptr<widget> ptr2{&object};
```

Characteristics:

- must always contain a valid address
- can not be default constructed
- can not be assigned `nullptr`
- supports reference binding
- supports raw pointer binding
- supports compatible pointer-like types
- supports type-erased `void` pointee types
- does not support pointer arithmetic

`required_ptr` models a required non-owning relationship while still allowing interoperability with traditional pointer-oriented APIs.

Typical use cases include:

- non-null function parameters
- object relationships
- aliases into externally owned storage
- interoperation with legacy pointer-based interfaces

---

### Nullable Object-Alias (`alias_ptr`)
`base.vocab.ptr:alias_ptr`

A general-purpose, nullable object-alias pointer.

```cpp
alias_ptr<widget> ptr;

if (!ptr) {
    ptr = some_widget;
}
```

Characteristics:

- may be empty
- supports default construction
- supports `nullptr`
- supports reference binding
- supports raw pointer binding
- supports compatible pointer-like types
- supports type-erased `void` pointee types
- does not support pointer arithmetic

`alias_ptr` models an optional non-owning relationship.

Typical use cases include:

- optional references
- observer relationships
- cached lookups
- delayed binding

---

### Memory Cursor (`cursor_ptr`)
`base.vocab.ptr:cursor_ptr`

A universal memory-cursor pointer.

```cpp
std::vector<int> values{1, 2, 3, 4};

cursor_ptr current{values.begin()};

++current;
```

Characteristics:

- must always contain a valid address
- can not be default constructed
- can not be assigned `nullptr`
- supports reference binding
- supports raw pointer binding
- supports compatible pointer-like types
- supports pointer arithmetic
- models contiguous iterator semantics

Unlike the aliasing vocabulary pointer types, `cursor_ptr` treats the stored address as a traversal position rather than merely an object association.

Typical use cases include:

- iterator-like traversal
- contiguous memory navigation
- buffer processing
- cursor-oriented algorithms

The distinction between `cursor_ptr` and the aliasing vocabulary pointer types is primarily its arithmetic traversal policy, which enables arithmetic operators, ordering comparisons, and iterator facilities.

---

### STL Iterator (`iterator_ptr`)
`base.vocab.ptr:iterator_ptr`

A STL-compatible nullable-iterator pointer.

Unlike `cursor_ptr`, `iterator_ptr` is nullable so that it satisfies the default-initializability requirements of `std::contiguous_iterator` and can serve directly as the iterator type of STL-compatible containers.

```cpp
template<typename T>
class my_vector {
public:
    using iterator = iterator_ptr<T>;

    iterator begin() noexcept;
    iterator end() noexcept;
    //...
};

my_vector<int> values{1, 2, 3, 4};

auto pos = std::ranges::find(values, 3);
```

Characteristics:

- may be empty
- supports default construction
- supports `nullptr`
- supports reference binding
- supports raw pointer binding
- supports compatible pointer-like types
- supports pointer arithmetic
- models `std::contiguous_iterator`

Unlike the aliasing vocabulary pointer types, `iterator_ptr` treats the stored address as a traversal position rather than merely an object association.

Typical use cases include:

- iterator-like traversal
- STL algorithm compatibility
- STL-like container iteration
- `std::ranges` interoperability

The distinction between `iterator_ptr` and the aliasing vocabulary pointer types is primarily its arithmetic traversal policy, which enables the arithmetic operators and ordering comparisons required by `std::contiguous_iterator`.

---

### Summary

|                  |          | Reference | Pointer | Arithmetic |
| Pointer Type     | Nullable | Binding   | Binding | Traversal  |
|------------------|----------|-----------|---------|------------|
| `dependency_ptr` | No       | Yes       | No      | No         |
| `required_ptr`   | No       | Yes       | Yes     | No         |
| `alias_ptr`      | Yes      | Yes       | Yes     | No         |
| `cursor_ptr`     | No       | Yes       | Yes     | Yes        |
| `iterator_ptr`  | Yes      | Yes       | Yes     | Yes        |

Each concrete vocabulary pointer type is uniquely distinguished by its selections among these four policies.

The remainder of this document describes the policy framework and core implementation machinery used to synthesize these concrete pointer types.

---
## Pointer Policies (`:policies`)
---

The `base.vocab.ptr:policies` partition defines the policy vocabulary used by `ptr_core` to synthesize pointer behavior at compile time. Rather than hard-coding semantics into individual pointer types, the library models pointer behavior as a collection of independent policy selections spanning several behavioral axes.

A valid pointer configuration selects **exactly one policy from each policy group**, producing a complete and unambiguous description of a pointer's capabilities and invariants.

### Policy Groups

The policy system consists of four independent behavioral axes:

|    Policy Group     |           Policies           |                                               Purpose                                              |
|---------------------|------------------------------|----------------------------------------------------------------------------------------------------|
| `traversal`         | `arithmetic`, `rebinding`    | Controls whether the pointer behaves as an iterator or only supports rebinding to other addresses. |
| `reference_binding` | `allowed`, `forbidden`       | Controls construction and assignment from lvalue references.                                       |
| `pointer_binding`   | `allowed`, `forbidden`       | Controls construction and assignment from other pointers (raw pointers or pointer-like types).     |
| `nullability`       | `nullable`, `always_engaged` | Controls whether the pointer can represent a null/empty/disengaged state.                          |

### Traversal Policies

#### `traversal::arithmetic`

Enables iterator-style arithmetic traversal semantics.

Pointers using this policy support:

- pointer arithmetic (`++`, `--`, `+=`, `-=`, `+`, `-`, etc.)
- address ordering comparisons (`<`, `<=`, `>`, `>=`)
- contiguous iterator semantics

This policy is appropriate for cursor- or iterator-like pointer vocabulary types.

#### `traversal::rebinding`

Enables traversal only through rebinding such as following linked list nodes.

Pointers using this policy:

- may be reassigned to different addresses
- do **not** support pointer arithmetic
- do **not** support address ordering comparisons
- are **not** iterators

This is the preferred policy for most non-owning object references where pointer arithmetic would be semantically meaningless.

---

### Reference Binding Policies

#### `reference_binding::allowed`

Allows construction and assignment directly from lvalue references.

```cpp
widget w;

required_ptr<widget> p{w};
p = w;
```

Binding from temporaries remains prohibited to reduce opportunities for dangling pointers.

#### `reference_binding::forbidden`

Disables construction and assignment from references.

Pointers using this policy must obtain their target addresses through alternative mechanisms.

---

### Pointer Binding Policies

#### `pointer_binding::allowed`

Allows construction and assignment from raw pointers and compatible pointer-like types exposing a suitable `get()` interface. This includes, for example, other vocabulary pointers such as pointing a `required_ptr` to the object at the current position of a `cursor_ptr` or binding a `required_ptr` to the address stored in an `alias_ptr` (after validating it is not null).

```cpp
widget object;
required_ptr<widget> p1{&object}; // raw pointer

auto owner = std::make_unique<widget>();
required_ptr<widget> p2{owner}; // pointer-like type
```

Pointer-like types are treated as first-class binding sources and participate in the same initialization and rebinding facilities as raw pointers.

#### `pointer_binding::forbidden`

Disables construction and assignment from pointer values.

This can be useful when a pointer type wishes to enforce stronger initialization rules while still allowing other forms of binding.

---

### Nullability Policies

#### `nullability::nullable`

Permits an empty state.

Pointers using this policy may:

- be default constructed
- be assigned `nullptr`
- release their target
- participate meaningfully in contextual conversion to `bool`

```cpp
alias_ptr<widget> p;

if (p) {
    // engaged
}
```

#### `nullability::always_engaged`

Enforces a non-null invariant.

Pointers using this policy:

- cannot be default constructed
- cannot be assigned `nullptr`
- must remain bound to a valid address for their entire lifetime

This policy forms the foundation of the library's non-null pointer vocabulary types.

---

### Defining Policy Lists

Policy configurations are represented as type sequences.

```cpp
using policies = type_list<
    traversal::rebinding,
    reference_binding::allowed,
    pointer_binding::allowed,
    nullability::always_engaged
>;
```

### Policy Groups as First-Class Types

Each policy exposes a nested `policy_group` alias identifying the behavioral axis to which it belongs.

```cpp
namespace traversal {
    struct group;

    struct arithmetic {
        using policy_group = group;
    };
}
```

This allows generic code to reason about policies by category (e.g., `traversal::group`) rather than by concrete type and enables compile-time validation of policy lists.

The canonical set of policy groups is exposed as `policy_groups`, and individual group markers satisfy `PtrPolicyGroup`.

Together, these facilities provide the foundation for policy validation, lookup, and compile-time dispatch throughout the pointer vocabulary implementation.

---

#### Compile-Time Policy List Validation

To ensure that concrete pointer configurations are well-formed and logically sound, a valid policy list **must** contain **exactly one policy from every policy group**.

This validation architecture is implemented through three nested compile-time primitives that compose to prevent compilation of invalid combinations:

1. `in_policy_group`: An internal meta-programming predicate that determines whether a single given policy belongs to a targeted `PtrPolicyGroup` by checking its nested `policy_group` alias.
2. `exactly_one_policy_v`: A variable template that maps the `in_policy_group` predicate across a complete user-supplied type sequence (such as a `type_list`), computing whether the count of matching policies for that category is exactly equal to `1`.
3. `valid_policy_list`: The top-level structural validator that takes the collection of all valid policy axes and folds over them using a binary right fold conjunction (`&&`). This guarantees that all canonical axes are satisfied simultaneously without any structural omissions or conflicting states (e.g., a type claiming to be both `nullable` and `always_engaged` or one that fails to select either).

The `PtrPolicyList` `concept` employs `valid_policy_list` to validate this requirement over a given `type_list`:

```cpp
template<PtrPolicyList Policies>
```

Invalid configurations are rejected during template instantiation.

---

### Querying Active Policies

The partition provides facilities for inspecting policy selections during template instantiation.

#### Retrieving the selected policy for a group

```cpp
using traversal_policy =
    group_policy_t<policies, traversal::group>;
```

#### Boolean policy queries

The library also exposes convenient compile-time predicates:

```cpp
arithmetic_traversal_v<Policies>
rebinding_traversal_v<Policies>

allowed_reference_binding_v<Policies>
forbidden_reference_binding_v<Policies>

allowed_pointer_binding_v<Policies>
forbidden_pointer_binding_v<Policies>

nullable_nullability_v<Policies>
always_engaged_nullability_v<Policies>
```

Example:

```cpp
if constexpr (always_engaged_nullability_v<Policies>) {
    // Non-null behavior
}
```

These traits are used extensively by `ptr_core` to selectively enable operations, enforce invariants, and synthesize the public interface of the various pointer vocabulary types.

---
## Core Pointer Definition (`:core`)
---

The `base.vocab.ptr:core` partition contains the policy realization engine used by all vocabulary pointer types.

While `:policies` defines the semantic vocabulary used to describe pointer behavior, `ptr_core` is responsible for translating those policy selections into a concrete pointer interface. It provides a common implementation foundation that stores addresses, enforces invariants, exposes pointer operations, and selectively enables or removes functionality according to the active policy configuration.

Concrete vocabulary pointer types are therefore primarily policy declarations layered on top of a shared implementation framework.

### Architectural Role

Conceptually, `ptr_core` sits between policy selection and the final vocabulary pointer type:

```text
Policy Configuration
        │
        ▼
     ptr_core
        │
        ▼
Concrete Pointer Type
```

The policy system specifies **what semantic contract a pointer should satisfy**, while `ptr_core` determines **which operations, constructors, assignments, comparisons, and invariants are required to realize that contract**.

---

### Core Representation

Regardless of policy configuration, every pointer vocabulary type ultimately stores a single memory address:

```cpp
address_type address_;
```

This address is the sole runtime state maintained by `ptr_core`.

All policy-driven behavior is resolved entirely at compile time, allowing different pointer vocabularies to expose distinct interfaces and invariants while preserving the storage characteristics of a raw pointer.

The implementation is designed to preserve standard-layout properties so that concrete vocabulary pointers remain structurally equivalent to a scalar pointer representation.

---

### Universal Pointer Facilities

All vocabulary pointers inherit a common set of core pointer operations independent of policy configuration.

These facilities expose the stored address and provide the fundamental mechanics required by higher-level pointer vocabularies:

- `operator->`
- `operator*`
- `get()`
- implicit conversion to the type of the stored address
- `reset(P)`
- `swap()`

These operations form the common vocabulary pointer interface. Policy selections then extend, restrict, or remove behavior around construction, assignment, traversal, comparison, and nullability.

As a result, every vocabulary pointer shares a consistent operational foundation while still presenting a distinct semantic contract.

---

### CRTP-Like Derived Type Recovery

`ptr_core` is parameterized by the concrete pointer template and the pointee element type:

```cpp
template<
    template<typename> typename ConcretePtr,
    typename Pointee,
    PtrPolicyList PolicySet
>
class ptr_core;
```

This allows operations implemented within `ptr_core` to construct, parameterize on, return, compare, and manipulate the final vocabulary type (`ConcretePtr<Pointee>`) rather than the base implementation type (`ptr_core`).

For example, arithmetic traversal operations return instances of the concrete pointer type, preserving the public vocabulary interface while centralizing implementation logic.

---

### Policy-Driven Interface Synthesis

Rather than maintaining separate implementations for every vocabulary pointer, `ptr_core` queries the active policy configuration and conditionally exposes behavior.

Examples include:

|          Policy Query          |                       Affected Facilities                        |
|--------------------------------|------------------------------------------------------------------|
| `arithmetic_traversal_v`       | Pointer arithmetic, iterator support, ordering comparisons       |
| `rebinding_traversal_v`        | Equality-only comparison semantics                               |
| `allowed_reference_binding_v`  | Construction and assignment from references                      |
| `allowed_pointer_binding_v`    | Construction and assignment from raw and pointer-like types      |
| `nullable_nullability_v`       | Default construction, `nullptr`, `release()`, engagement testing |
| `always_engaged_nullability_v` | Non-null invariants and null-state elimination                   |

This synthesis occurs entirely during template instantiation and introduces no runtime overhead.

---

### Enabling vs. Deleting Operations

A notable design characteristic of `ptr_core` is that unsupported operations are frequently represented by deleted overloads rather than simply being omitted.

For example:

- Rebinding traversal pointers explicitly delete arithmetic operators.
- Always-engaged pointers explicitly delete `nullptr` construction and assignment.
- Reference-forbidden pointers explicitly delete reference binding operations.
- Pointer-forbidden pointers explicitly delete pointer binding operations.

This approach improves diagnostics by allowing overload resolution to identify the semantically correct operation before reporting that the operation is prohibited by policy.

In many cases, deleted overloads are intentionally preferred over value-category failures so that policy violations are diagnosed directly.

---

### Structural Safety Enforcement

Several library-wide safety rules are enforced within `ptr_core` independently of the active pointer vocabulary.

#### Temporary Binding Prevention

Direct aliasing of temporary objects is structurally prohibited.

This includes:

- binding from rvalue references
- binding from temporary pointer-like objects
- assignment from temporary pointer-like objects

These deleted overloads eliminate a common source of dangling pointers at the vocabulary boundary.

#### Array Decay Prevention

Raw C-array parameters are explicitly intercepted and deleted to prevent array-to-pointer decay and force callers to explicitly identify the intended element address. This restriction targets **array-to-pointer decay**, not array objects themselves.

```cpp
int values[10];

required_ptr<int>     decay_ptr{values}; //error: decay
required_ptr<int[10]> array_ptr{values}; //ok: pointer-to-array

int not_good[10][10];

required_ptr<int[10]> bad_ptr{not_good}; //error: decay
```

In the second case, the array object itself is the pointee, so no decay occurs. In the third case, the outer array would decay to a raw pointer, which makes the construction ill-formed.

---

### Explicit Object Parameter Architecture

The implementation makes extensive use of C++23 explicit object parameters:

```cpp
template<typename Self>
constexpr Self& operator=(this Self& self, reference source);
```

This allows a single implementation to correctly propagate cv-qualification and value category without requiring separate overload sets for:

- mutable vs. const objects
- lvalues vs. rvalues
- derived concrete pointer specializations

The result is a substantially smaller implementation surface while preserving correct type behavior.

---

### Standard Library Integration

All vocabulary pointer types automatically participate in standard pointer traits, hashing, formatting, and common reference facilities.

#### Pointer Traits

`std::pointer_traits` is specialized for all `VocabPtr` types. This includes provision of pointer, element, and difference types as well as the `rebind` template, the `pointer_to` factory, and the `to_address` function.

```cpp
template<typename P>
auto pointer_to_const(typename std::pointer_traits<P>::element_type& object) {
    return std::pointer_traits<P>::template rebind<std::add_const_t<typename std::pointer_traits<P>::element_type>>::pointer_to(object);
}

std::int32_t value = 42;
auto ptr = pointer_to_const<required_ptr<std::int32_t>>(value);
assert(std::to_address(ptr) == std::addressof(value));
```

#### Hashing

`std::hash` is specialized for all `VocabPtr` types:

```cpp
std::unordered_set<required_ptr<widget>> widgets;
```

Like all comparison facilities provided by `ptr_core`, hashing operates on pointer identity (stored addresses) rather than pointee object values.

This behavior aligns with the semantic model that vocabulary pointers compare and identify addresses rather than the state of the objects to which they point.

#### Formatting

`std::formatter` is specialized for all `VocabPtr` types:

```cpp
std::println("{}", ptr);
```

Vocabulary pointers format identically to raw pointers by formatting the stored address according to the rules for `const void*`.

This allows vocabulary pointers to participate naturally in the C++ formatting ecosystem without requiring explicit extraction of the stored address as a raw pointer.

#### Stream Output

Vocabulary pointers also provide stream insertion support:

```cpp
std::cout << ptr;
```

Like formatting and hashing, stream output operates on the stored address rather than the pointee object.

Together, these facilities allow vocabulary pointers to integrate naturally with standard containers, formatting libraries, logging systems, and diagnostic tooling while preserving pointer-identity semantics.

#### Common Reference

`std::basic_common_reference` is specialized for cross-type comparisons between different `VocabPtr` types yielding the raw address type as their common reference. Similarly, specializations produce the common raw address type as the common reference between a `VocabPtr` and a raw pointer.

---

### Relationship to Concrete Pointer Types

Concrete vocabulary pointers generally contribute no runtime behavior of their own. Instead, they select a policy configuration and inherit the corresponding interface synthesized by `ptr_core`. As a result, introducing a new vocabulary pointer type is an exercise in declaring a new semantic policy configuration rather than implementing a new pointer abstraction from scratch.
