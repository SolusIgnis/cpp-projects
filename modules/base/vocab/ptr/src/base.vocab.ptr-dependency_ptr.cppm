// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-dependency_ptr.cppm
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
 * @brief `dependency_ptr`: A compile-time validated, reference-bound, required-dependency alias pointer type.
 *
 * @details This pointer provides a zero-overhead core vocabulary primitive
 * for dependency injection, modeling a required-dependency alias with standard
 * pointer ergonomics. It expresses a mandatory dependency that must outlive
 * its consumer. It is intended to replace non-static reference data members to
 * preserve class assignability. It also directly replaces raw pointers to
 * dependency objects and provides an alternative to `std::reference_wrapper`.
 *
 * By utilizing structural constraints (specifically restricting initialization
 * and rebinding exclusively to references to the pointee), it enforces validity
 * entirely at compilation. Unlike `required_ptr`, it completely eliminates the
 * necessity and cost of runtime validation. Unlike `cursor_ptr`, it operates
 * solely on discrete object identities and rejects pointer arithmetic entirely.
 *
 * To represent a contextually optional dependency, it composes naturally as 
 * `std::optional<dependency_ptr<T>>`, cleanly decoupling the optionality from
 * the pointer itself. 
 */

//Module partition interface unit
export module base.vocab.ptr:dependency_ptr;

import std;

import base.meta.traits;
import base.meta.concepts;

import :core;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type aliasing a required dependency.
     *
     * @tparam Pointee The type of the required dependency. Must be a non-void valid pointee type (non-reference and no degree of indirection over a function).
     *
     * @details `dependency_ptr` models a non-owning, always-engaged, non-arithmetic, non-void-permitting
     * pointer abstraction that supports rebinding traversal and only binds from object references rather
     * than other pointers. It is designed for dependency injection scenarios, where a dependency is
     * required to exist and outlive the consumer. It serves as a replacement for references as non-static
     * data members and in other cases where rebinding, pointer semantics, or interoperability with
     * pointer-based APIs is desirable. Optional dependencies naturally compose with `dependency_ptr` as
     * `std::optional<dependency_ptr<Pointee>>`.
     *
     * The invariants and behavioral contract of `dependency_ptr` are determined by the policies selected
     * in the `ptr_core` specialization from which it is derived:
     * - `nullability::always_engaged`
     * - `pointer_binding::forbidden`
     * - `reference_binding::allowed`
     * - `traversal::rebinding`
     *
     * @see `ptr_core`
     * @see `ptr_policies`
     *
     * @note Semantically equivalent to a rebindable reference with a pointer interface.
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `dependency_ptr` always stores a valid memory address; there is no null/disengaged representation.
     * @note The stored address is guaranteed to be non-null by construction without need for runtime validation.
     * @remark This type does not own the pointee object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the pointer without affecting either pointee.
     * @remark Marked `[[nodiscard]]` to prevent accidental dropping of required dependency handles. Intentional discards should use `[[maybe_unused]]` to document intent.
     * @note All operations provide the No-Fail Guarantee, performing exclusively non-allocating, non-throwing pointer manipulations.
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
        using base_type = typename dependency_ptr::ptr_core;
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
