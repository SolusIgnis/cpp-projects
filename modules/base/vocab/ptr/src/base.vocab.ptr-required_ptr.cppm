// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-required_ptr.cppm
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
 * @brief `required_ptr`: A general-purpose, void-permitting, required-object alias pointer type.
 *
 * @details This pointer provides a zero-overhead replacement for raw
 * pointers as a core vocabulary type for non-owning object aliases. It
 * should be preferred over `alias_ptr` in contexts where the alias is
 * required to point to a valid memory address as a precondition or
 * invariant since it enforces that validity at the type boundary to
 * avoid unnecessary revalidation.
 *
 * By utilizing a mix of structural constraints and runtime validation,
 * it enforces engagement at the point of construction or rebinding.
 * Unlike `dependency_ptr`, `required_ptr` supports fluid initialization
 * and rebinding from other pointers, increasing its versatility. Unlike
 * `cursor_ptr`, it is unsuitable for arithmetic iteration over blocks of
 * contiguous memory; rather, it is suited to discrete object-to-object
 * rebinding path traversal (such as linked nodes).
 */

//Module partition interface unit
export module base.vocab.ptr:required_ptr;

import std;

import base.meta.traits;
import base.meta.concepts;

import :core;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type aliasing a required object.
     *
     * @tparam Pointee The pointed-to type. Must be a valid pointee type (non-reference and no degree of indirection over a function).
     *
     * @details `required_ptr` models a non-owning, always-engaged, non-arithmetic, void-permitting
     * pointer abstraction that supports rebinding traversal and may bind from object references as
     * well as other pointers. It is designed as a general-purpose object alias in scenarios where
     * the aliased object is required to exist and outlive the alias. By enforcing address validity
     * at the encapsulation boundary, it saves downstream consumers from branching to verify at the
     * point of dereference while maintaining full initialization and rebinding interoperability with
     * other pointers.
     *
     * The invariants and behavioral contract of `required_ptr` are determined by the policies selected
     * in the `ptr_core` specialization from which it is derived:
     * - `nullability::always_engaged`
     * - `pointer_binding::allowed`
     * - `reference_binding::allowed`
     * - `traversal::rebinding`
     *
     * @see `ptr_core`
     * @see `ptr_policies`
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `required_ptr` always stores a valid memory address; there is no null/disengaged representation.
     * @remark Initialization or rebinding from other pointers performs runtime validation and throws `std::invalid_argument` when a null value is supplied.
     * @remark This type does not own the pointee object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the pointer without affecting either pointee.
     * @note Supports type-erased aliasing by permitting `void` pointees through the primary template rather than introducing a specialization. Placeholder reference aliases based on a private nested incomplete type are used solely to keep deleted overload declarations well-formed.
     * @note Construction and assignment/rebinding from other pointers provide the Strong Exception Safety Guarantee. All other operations provide the No-Fail Guarantee, performing exclusively non-allocating, non-throwing pointer manipulations.
     *
     * @warning The referenced object MUST outlive the `required_ptr`. Violating this results in undefined behavior.
     *
     * @see `alias_ptr` for nullable aliasing, `dependency_ptr` for dependency injection structural non-nullability, `cursor_ptr` for non-null arithmetic traversal, `iterator_ptr` for nullable arithmetic traversal, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename Pointee>
        requires is_valid_pointee_v<Pointee>
    class required_ptr final : public ptr_core<
                                   required_ptr,
                                   Pointee,
                                   ptr_policies::type_list<
                                       ptr_policies::nullability::always_engaged,
                                       ptr_policies::pointer_binding::allowed,
                                       ptr_policies::reference_binding::allowed,
                                       ptr_policies::traversal::rebinding
                                   >
                               > {
    private:
        using base_type = required_ptr::core_type;

    public:
        using base_type::base_type;
        using base_type::operator=;
    };

    /**
     * @brief Deduction guide for `required_ptr` from references.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     * @remark Excludes `pointer_with_element_type`s to avoid ambiguity with other CTAD guides.
     * @remark Excludes non-array `resolvable_to_address`s to avoid ambiguity with other CTAD guides.
     */
    template<typename T>
        requires (!pointer_with_element_type<T>) && ((!resolvable_to_address<T>) || std::is_array_v<std::remove_cvref_t<T>>)
    required_ptr(T&&) -> required_ptr<std::remove_reference_t<T>>;

    /**
     * @brief Deduction guide for `required_ptr` from pointers.
     *
     * @tparam P A pointer-like type with an `element_type` exposed through `std::pointer_traits`.
     */
    template<pointer_with_element_type P>
    required_ptr(P) -> required_ptr<typename std::pointer_traits<P>::element_type>;

    /**
     * @brief Deduction guide for `required_ptr` from address-resolvable types.
     *
     * @tparam P A type resolvable by `std::to_address`.
     *
     * @remark Excludes `pointer_with_element_type`s to avoid ambiguity with other CTAD guides.
     * @remark Excludes C arrays to avoid array-to-pointer decay in the type deduction.
     */
    template<resolvable_to_address P>
        requires (!pointer_with_element_type<P>) && (!std::is_array_v<P>)
    required_ptr(P) -> required_ptr<address_resolved_element_t<P>>;
} //namespace base::vocab::inline ptr
