// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-cursor_ptr.cppm
 * @version 0.6.0
 * @date June 9, 2026
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
 * @brief `cursor_ptr`: A universal memory-cursor pointer type.
 *
 * @details This pointer provides a zero-overhead core vocabulary primitive
 * for contiguous memory traversal. It is intended to replace raw pointers
 * as the memory cursor underlying random-access and contiguous iterators.
 *
 * By utilizing a mix of structural constraints and runtime validation,
 * it enforces engagement at the point of construction or rebinding.
 * Unlike `dependency_ptr`, `required_ptr`, and `alias_ptr`, it is suited
 * to arithmetic iteration over blocks of contiguous memory rather than
 * discrete object-to-object rebinding path traversal. However, it still
 * offers full pointer- and reference-binding capabilities for fluid
 * interoperability and cross-sequence rebinding.
 */

//Module partition interface unit
export module base.vocab.ptr:cursor_ptr;

import std;

import base.meta.traits;
import base.meta.concepts;

import :core;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a contiguous memory cursor.
     *
     * @tparam Pointee The pointed-to type. Must be a non-void valid pointee type (non-reference and no degree of indirection over a function).
     *
     * @details `cursor_ptr` models a non-owning, always-engaged, arithmetic, non-void-
     * permitting pointer abstraction. It is designed for random-access and contiguous
     * iteration over objects stored in contiguous memory. Because it exposes memory
     * traversal semantics (pointer arithmetic and ordering), it serves as a cursor into
     * a sequence rather than a stable alias to a single object.
     *
     * The invariants and behavioral contract of `cursor_ptr` are determined by the policies selected
     * in the `ptr_core` specialization from which it is derived:
     * - `nullability::always_engaged`
     * - `pointer_binding::allowed`
     * - `reference_binding::allowed`
     * - `traversal::arithmetic`
     *
     * @see `ptr_core`
     * @see `ptr_policies`
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `cursor_ptr` always stores a valid memory address; there is no null/disengaged representation.
     * @remark Initialization or rebinding from other pointers performs runtime validation and throws `std::invalid_argument` when a null value is supplied.
     * @remark This type does not own the pointee object and does not participate in lifetime management.
     * @remark Copying and assignment perform potentially cross-sequence rebinding of the pointer without affecting either sequence.
     * @note Because arithmetic traversal requires a complete type to compute `sizeof(Pointee)`, `void` pointees are semantically incompatible.
     * @note Construction and assignment/rebinding from other pointers provide the Strong Exception Safety Guarantee. All other operations provide the No-Fail Guarantee, performing exclusively non-allocating, non-throwing pointer manipulations.
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
        using base_type = typename cursor_ptr::ptr_core;
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
