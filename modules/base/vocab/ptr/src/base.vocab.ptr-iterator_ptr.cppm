// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-iterator_ptr.cppm
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
 * @brief `iterator_ptr`: A STL-compatible nullable-iterator pointer type.
 *
 * @details This pointer provides a zero-overhead core vocabulary primitive
 * for STL-compatible contiguous iteration. It is intended to replace raw
 * pointers as the memory cursor underlying random-access and contiguous
 * iterators into STL-style containers and range-based algorithms.
 *
 * Unlike `cursor_ptr`, `iterator_ptr` models a nullable (optionally
 * engaged) pointer concept and defers address validation to explicit
 * engagement checks at the point of use. This enables default (null)
 * initialization, fulfilling the `std::regular` requirements necessary to
 * model the standard iterator concepts for generic range-based algorithms.
 * Unlike `dependency_ptr`, `required_ptr`, and `alias_ptr`, it is suited
 * to arithmetic iteration over blocks of contiguous memory rather than
 * discrete object-to-object rebinding path traversal. However, it still
 * offers full pointer- and reference-binding capabilities for fluid
 * interoperability and relocation of the cursor into a different
 * contiguous memory sequence.
 */

//Module partition interface unit
export module base.vocab.ptr:iterator_ptr;

import std;

import base.meta.traits;
import base.meta.concepts;

import :core;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a nullable contiguous memory iterator.
     *
     * @tparam Pointee The pointed-to type. Must be a non-void valid pointee type (non-reference and no degree of indirection over a function).
     *
     * @details `iterator_ptr` models a non-owning, nullable, arithmetic, non-void-permitting
     * pointer abstraction. It is designed for random-access and contiguous iteration over
     * contiguous ranges. Because it exposes memory traversal semantics (pointer arithmetic
     * and ordering), it serves as an iterator into a sequence rather than a stable alias to
     * a single object.
     *
     * The invariants and behavioral contract of `iterator_ptr` are determined by the policies selected
     * in the `ptr_core` specialization from which it is derived:
     * - `nullability::nullable`
     * - `pointer_binding::allowed`
     * - `reference_binding::allowed`
     * - `traversal::arithmetic`
     *
     * @see `ptr_core`
     * @see `ptr_policies`
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @remark This type does not own the pointee object and does not participate in lifetime management.
     * @remark Copying and assignment perform rebinding, potentially to a different contiguous sequence, of the pointer without affecting either sequence.
     * @note Because arithmetic traversal requires a complete type to compute `sizeof(Pointee)`, `void` pointees are semantically incompatible.
     * @note All operations provide the No-Fail Guarantee, performing exclusively non-allocating, non-throwing pointer manipulations.
     *
     * @warning The referenced object MUST outlive the `iterator_ptr`. Violating this results in undefined behavior.
     * @warning Any operation that invalidates the underlying contiguous storage also invalidates associated `iterator_ptr` instances.
     *
     * @see `cursor_ptr` for non-null arithmetic traversal, `alias_ptr` for nullable aliasing, `required_ptr` for non-null aliasing, `dependency_ptr` for dependency injection structural non-nullability, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename Pointee>
        requires is_valid_pointee_v<Pointee> && (!std::is_void_v<Pointee>)
    class iterator_ptr final : public ptr_core<
                                   iterator_ptr,
                                   Pointee,
                                   ptr_policies::type_list<
                                       ptr_policies::nullability::nullable,
                                       ptr_policies::pointer_binding::allowed,
                                       ptr_policies::reference_binding::allowed,
                                       ptr_policies::traversal::arithmetic
                                   >
                               > {
    private:
        using base_type = iterator_ptr::core_type;

    public:
        using base_type::base_type;
        using base_type::operator=;
    };

    /**
     * @brief Deduction guide for `iterator_ptr` from references.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     * @remark Excludes `pointer_with_element_type`s to avoid ambiguity with other CTAD guides.
     * @remark Excludes non-array `resolvable_to_address`s to avoid ambiguity with other CTAD guides.
     */
    template<typename T>
        requires (!pointer_with_element_type<T>) && ((!resolvable_to_address<T>) || std::is_array_v<std::remove_cvref_t<T>>)
    iterator_ptr(T&&) -> iterator_ptr<std::remove_reference_t<T>>;

    /**
     * @brief Deduction guide for `iterator_ptr` from pointers.
     *
     * @tparam P A pointer-like type with an `element_type` exposed through `std::pointer_traits`.
     */
    template<pointer_with_element_type P>
    iterator_ptr(P) -> iterator_ptr<typename std::pointer_traits<P>::element_type>;

    /**
     * @brief Deduction guide for `iterator_ptr` from address-resolvable types.
     *
     * @tparam P A type resolvable by `std::to_address`.
     *
     * @remark Excludes `pointer_with_element_type`s to avoid ambiguity with other CTAD guides.
     * @remark Excludes C arrays to avoid array-to-pointer decay in the type deduction.
     */
    template<resolvable_to_address P>
        requires (!pointer_with_element_type<P>) && (!std::is_array_v<P>)
    iterator_ptr(P) -> iterator_ptr<address_resolved_element_t<P>>;
} //namespace base::vocab::inline ptr
