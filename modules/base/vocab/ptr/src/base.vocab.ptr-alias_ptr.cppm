// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-alias_ptr.cppm
 * @version 0.7.0
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
 * @brief `alias_ptr`: A general-purpose, void-permitting, nullable object-alias pointer type.
 *
 * @details This pointer provides a zero-overhead replacement for raw
 * pointers as a core vocabulary type for non-owning object aliases. It
 * should be preferred over `required_ptr` in contexts where an unengaged
 * (null) state meaningfully exists, cleanly representing optionality
 * without introducing a wrapper.
 *
 * Unlike `dependency_ptr` and `required_ptr`, `alias_ptr` models a
 * nullable (optionally engaged) pointer concept and defers address
 * validation to explicit engagement checks at the point of use. It
 * fully supports fluid initialization and rebinding from other pointers
 * without the overhead of a null-check at the encapsulation boundary.
 * Unlike `cursor_ptr`, it is unsuitable for arithmetic iteration over
 * blocks of contiguous memory; rather, it is suited to discrete object-
 * to-object rebinding path traversal (such as linked nodes).
 */

//Module partition interface unit
export module base.vocab.ptr:alias_ptr;

import std;

import base.meta.traits;
import base.meta.concepts;

import :core;

export namespace base::vocab::inline ptr {
    /**
     * @brief Nullable pointer type aliasing an object.
     *
     * @tparam Pointee The pointed-to type. Must be a valid pointee type (non-reference and no degree of indirection over a function).
     *
     * @details `alias_ptr` models a non-owning, nullable, non-arithmetic, void-permitting pointer
     * abstraction that supports rebinding traversal and may bind from object references as well as
     * other pointers. It is designed as a general-purpose object alias in scenarios where there may
     * be no valid pointee object. The pointer's owner has the responsibility to track the pointee
     * lifetime and update the pointer accordingly.
     *
     * The invariants and behavioral contract of `alias_ptr` are determined by the policies selected
     * in the `ptr_core` specialization from which it is derived:
     * - `nullability::nullable`
     * - `pointer_binding::allowed`
     * - `reference_binding::allowed`
     * - `traversal::rebinding`
     *
     * @see `ptr_core`
     * @see `ptr_policies`
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @remark This type does not own the pointee object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the pointer without affecting either pointee.
     * @note Supports type-erased aliasing by permitting `void` pointees through the primary template rather than introducing a specialization. Placeholder reference aliases based on a private nested incomplete type are used solely to keep deleted overload declarations well-formed.
     * @note All operations provide the No-Fail Guarantee, performing exclusively non-allocating, non-throwing pointer manipulations.
     *
     * @warning The referenced object MUST outlive the `alias_ptr`. Violating this results in undefined behavior.
     *
     * @see `required_ptr` for non-null aliasing, `dependency_ptr` for dependency injection structural non-nullability, `cursor_ptr` for non-null iteration/traversal, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename Pointee>
        requires is_valid_pointee_v<Pointee>
    class alias_ptr final : public ptr_core<
                                alias_ptr,
                                Pointee,
                                ptr_policies::type_list<
                                    ptr_policies::nullability::nullable,
                                    ptr_policies::pointer_binding::forbidden,
                                    ptr_policies::reference_binding::allowed,
                                    ptr_policies::traversal::arithmetic
                                >
                            > {
    private:
        using base_type = typename alias_ptr::core_type;

    public:
        using base_type::base_type;
        using base_type::operator=;
    };

    /**
     * @brief Deduction guide for `alias_ptr`.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     */
    template<typename T>
    alias_ptr(T&) -> alias_ptr<T>;

    /**
     * @brief Deduction guide for `alias_ptr`.
     *
     * @tparam T The type of the pointee.
     *
     * @remark Deduces `T` from the pointee, preserving cv-qualification.
     */
    template<typename T>
    alias_ptr(T*) -> alias_ptr<T>;
} //namespace base::vocab::inline ptr
