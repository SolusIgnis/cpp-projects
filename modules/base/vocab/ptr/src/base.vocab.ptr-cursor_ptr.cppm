// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-cursor_ptr.cppm
 * @version 0.9.1
 * @date August 21, 2026
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
 * interoperability and relocation of the cursor into a different
 * contiguous memory sequence.
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
     * @remark Copying and assignment perform rebinding, potentially to a different contiguous sequence, of the pointer without affecting either sequence.
     * @note Because arithmetic traversal requires a complete type to compute `sizeof(Pointee)`, `void` pointees are semantically incompatible.
     * @note Construction and assignment/rebinding from other pointers provide the Strong Exception Safety Guarantee. All other operations provide the No-Fail Guarantee, performing exclusively non-allocating, non-throwing pointer manipulations.
     *
     * @warning The referenced object MUST outlive the `cursor_ptr`. Violating this results in undefined behavior.
     * @warning Any operation that invalidates the underlying contiguous storage also invalidates associated `cursor_ptr` instances.
     *
     * @see `iterator_ptr` for nullable arithmetic traversal, `alias_ptr` for nullable aliasing, `required_ptr` for non-null aliasing, `dependency_ptr` for dependency injection structural non-nullability, `std::unique_ptr` and `std::shared_ptr` for ownership.
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
        using base_type = typename cursor_ptr::core_type;

    public:
        using base_type::base_type;
        using base_type::operator=;
    };

    /**
     * @brief Deduction guide for `cursor_ptr` from references.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     * @remark Excludes `PointerWithElementType`s to avoid ambiguity with other CTAD guides.
     * @remark Excludes non-array `ResolvableToAddress`s to avoid ambiguity with other CTAD guides.
     */
    template<typename T>
        requires (!PointerWithElementType<T>) && ((!ResolvableToAddress<T>) || std::is_array_v<std::remove_cvref_t<T>>)
    cursor_ptr(T&&) -> cursor_ptr<std::remove_reference_t<T>>;

    /**
     * @brief Deduction guide for `cursor_ptr` from pointers.
     *
     * @tparam P A pointer-like type with an `element_type` exposed through `std::pointer_traits`.
     */
    template<PointerWithElementType P>
    cursor_ptr(P) -> cursor_ptr<typename std::pointer_traits<P>::element_type>;

    /**
     * @brief Deduction guide for `cursor_ptr` from address-resolvable types.
     *
     * @tparam P A type resolvable by `std::to_address`.
     *
     * @remark Excludes `PointerWithElementType`s to avoid ambiguity with other CTAD guides.
     * @remark Excludes C arrays to avoid array-to-pointer decay in the type deduction.
     */
    template<ResolvableToAddress P>
        requires (!PointerWithElementType<P>) && (!std::is_array_v<P>)
    cursor_ptr(P) -> cursor_ptr<address_resolved_element_t<P>>;
} //namespace base::vocab::inline ptr
