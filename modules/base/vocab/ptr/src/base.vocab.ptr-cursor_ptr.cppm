// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-cursor_ptr.cppm
 * @version 0.6.0
 * @date May 7, 2026
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
 * @brief `cursor_ptr`: non-owning, non-nullable, arithmetic, non-void-permitting
 *
 * @details
 *
 * `cursor_ptr` provides a vocabulary pointer type representing a non-owning,
 * non-null cursor into contiguous memory. Unlike stable aliasing pointer
 * abstractions, `cursor_ptr` intentionally exposes traversal semantics through
 * pointer arithmetic, ordering, and iterator interoperability.
 *
 * This type models a lightweight random-access and contiguous iterator over
 * existing storage while preserving explicit non-ownership semantics. It is
 * intended for APIs where nullable states are invalid by construction and where
 * pointer traversal is semantically meaningful.
 *
 * The abstraction preserves the operational behavior of raw pointers while
 * constraining several historically error-prone language behaviors:
 * - Null construction and rebinding are prohibited structurally.
 * - Direct binding to temporaries is rejected to discourage dangling.
 * - C-array decay is rejected to prevent implicit loss of extent information.
 * - Ownership transfer semantics are intentionally absent.
 *
 * `cursor_ptr` is suitable for:
 * - Traversing contiguous object sequences.
 * - Expressing non-null iterator-like API contracts.
 * - Interoperating with low-level and legacy pointer-based interfaces.
 * - Generic code requiring contiguous or random-access iterator semantics.
 *
 * `cursor_ptr` is not intended to model:
 * - Ownership or lifetime management.
 * - Optional/disengaged pointer states.
 * - Stable aliases to a single object independent of surrounding storage.
 *
 * As with raw pointers and iterators, validity depends entirely on external
 * lifetime and storage guarantees. Operations that invalidate the underlying
 * contiguous storage also invalidate all associated `cursor_ptr` instances.
 *
 * @todo Future Development: Use `= delete("reason")` once the C++26 feature becomes available.
 */

//Module partition interface unit
export module base.vocab.ptr:cursor_ptr;

import std;

import base.meta.traits;
import base.meta.concepts;

import :core;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a non-null cursor/iterator.
     *
     * @tparam T The pointed-to type (must not be a reference type per "C++ standard [dcl.ptr]", `void`, nor a function pointer).
     *
     * @details `cursor_ptr` models a non-owning, non-nullable, arithmetic, non-void-permitting
     * pointer abstraction. It is designed for random-access and contiguous iteration over objects
     * stored in contiguous memory. Because it exposes memory traversal semantics (pointer
     * arithmetic and ordering), it is not a stable alias to a single object.
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `cursor_ptr` always stores a valid memory address; there is no null/disengaged representation. Attempts to construct/assign from another pointer throw on null.
     * @remark This type does not own the pointee object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the stored address without affecting pointee lifetime.
     * @note Construction or assignment from `nullptr` is ill-formed.
     * @note Construction or assignment from pointers and pointer-like values performs runtime validation and throws `std::invalid_argument` on null. All operations provide the Strong Exception Safety Guarantee.
     * @remark Explicit comparison overloads are provided only where implicit conversion to the nested `pointer` type is insufficient to enable the desired comparison.
     *
     * @warning The referenced object MUST outlive the `cursor_ptr`. Violating this results in undefined behavior.
     * @warning Any operation that invalidates the underlying contiguous storage also invalidates associated `cursor_ptr` instances.
     *
     * @see `alias_ptr` for nullable aliasing, `required_ptr` for non-null aliasing, `dependency_ptr` for dependency injection structural non-nullability, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename Pointee>
        requires is_valid_pointee_v<Pointee> && (!std::is_void_v<Pointee>)
    class cursor_ptr final : public ptr_core<
        cursor_ptr,
        Pointee,
        ptr_policies::type_list<
            ptr_policies::nullability::always_engaged,
            ptr_policies::pointer_binding::allowed,
            ptr_policies::reference_binding::allowed,
            ptr_policies::traversal::arithmetic
        >
    > {
    private:
        using base_type = cursor_ptr::ptr_core;
    public:
        using base_type::base_type;
        using base_type::operator=;
    };

    /**
     * @brief Deduction guide for `cursor_ptr`.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     */
    template<typename T>
    cursor_ptr(T&) -> cursor_ptr<T>;

    /**
     * @brief Deduction guide for `cursor_ptr`.
     *
     * @tparam T The type of the pointee.
     *
     * @remark Deduces `T` from the pointee, preserving cv-qualification.
     */
    template<typename T>
    cursor_ptr(T*) -> cursor_ptr<T>;
} //namespace base::vocab::inline ptr
