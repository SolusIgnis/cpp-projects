// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-dependency_ptr.cppm
 * @version 0.6.0
 * @date May 3, 2026
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
 * @brief `dependency_ptr`: A non-owning, non-nullable, non-arithmetic, non-void-permitting pointer type for a required dependency.
 *
 * @details `dependency_ptr` acts like a rebindable reference with pointer syntax. It
 * expresses a mandatory dependency that must exist for the duration of the consumer's
 * lifetime. It naturally composes as `std::optional<dependency_ptr<T>>` to represent
 * a contextually optional dependency decoupled from the pointer itself.
 *
 * By utilizing structural constraints (deleted constructors for `nullptr_t` and raw
 * `pointer`), it enforces validity at the point of construction. It is intended to replace:
 * - Non-static reference data members (which make a class non-assignable).
 * - Raw pointers used as dependencies (which are semantically muddy regarding optionality and ownership).
 * - `std::reference_wrapper` (when pointer-like `->` access is preferred).
 * - `required_ptr` or `alias_ptr` when used for dependency injection.
 *
 * @todo Future Development: Use `= delete("reason")` instead of the C-style comments once the C++26 feature becomes available.
 */

//Module partition interface unit
export module base.vocab.ptr:dependency_ptr;

import std;

import base.meta.traits;
import base.meta.concepts;

import :core;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a required dependency.
     *
     * @tparam T The pointed-to type (must not be a reference type per "C++ standard [dcl.ptr]", `void`, nor a function pointer).
     *
     * @details `dependency_ptr` models a non-owning, non-nullable, non-arithmetic, non-void-permitting
     * pointer abstraction. It is designed for dependency injection scenarios, where a dependency is
     * required to exist and outlive the consumer. It serves as a replacement for references as nonstatic
     * data members and in other cases where rebinding, pointer semantics, or interoperability with
     * pointer-based APIs is desirable. Optional dependencies naturally compose with `dependency_ptr` as
     * `std::optional<dependency_ptr<T>>`.
     *
     * @note Semantically equivalent to a rebindable reference with a pointer interface.
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `dependency_ptr` always refers to a valid object; there is no null/disengaged representation. The wrapped pointer is guaranteed to be non-null by construction.
     * @remark This type does not own the referenced object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the pointer without affecting the lifetime of the underlying object.
     * @note Construction or assignment from `nullptr` is ill-formed.
     * @remark Marked `[[nodiscard]]` to prevent accidental construction of unused dependency objects. Intentional discards should use `[[maybe_unused]]` to document intent.
     * @remark Construction requires an lvalue reference, preventing null initialization and discouraging dangling by rejecting direct binding to temporaries.
     * @note Non-arithmetic Pointer: Pointer arithmetic and ordering comparisons are intentionally disabled to prevent misuse as an iterator.
     * @remark Implicit conversion to raw pointer is provided for interoperability with legacy or low-level APIs.
     * @note All operations provide the no-throw guarantee; operations consist exclusively of non-throwing pointer manipulation.
     * @remark Explicit equality comparison overloads are provided only where built-in pointer comparison cannot be reached through the implicit raw-pointer conversion operator alone.
     *
     * @warning The referenced object MUST outlive the `dependency_ptr`. Violating this results in undefined behavior.
     *
     * @see `alias_ptr` for nullable aliasing, `required_ptr` for non-null aliasing, `cursor_ptr` for non-null iteration/traversal, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename Pointee>
        requires is_valid_pointee_v<Pointee> && (!std::is_void_v<Pointee>)
    class [[nodiscard]] dependency_ptr final : public ptr_core<
        dependency_ptr,
        Pointee,
        ptr_policies::type_list<
            ptr_policies::nullability::always_engaged,
            ptr_policies::pointer_binding::forbidden,
            ptr_policies::reference_binding::allowed,
            ptr_policies::traversal::rebinding
        >
    > {
    private:
        using base_type = dependency_ptr::ptr_core;
    public:
        using base_type::base_type;
        using base_type::operator=;
    };

    /**
     * @brief Deduction guide for `dependency_ptr`.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     */
    template<typename T>
    dependency_ptr(T&) -> dependency_ptr<T>;
} //namespace base::vocab::inline ptr
