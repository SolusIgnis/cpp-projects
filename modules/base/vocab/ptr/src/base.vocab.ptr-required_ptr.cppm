// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-required_ptr.cppm
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
 * @brief `required_ptr`: A non-owning, non-nullable, non-arithmetic, void-permitting pointer type.
 *
 * @details `required_ptr` is a vocabulary pointer type representing
 * a non-owning, structurally non-null object alias. It is intended to
 * replace raw pointers as non-owning object aliases and should be
 * preferred over `alias_ptr` in contexts where the alias is required
 * to be non-null as a precondition or invariant.
 *
 * Unlike raw pointers, `required_ptr` cannot be default-constructed,
 * assigned `nullptr`, or participate in pointer arithmetic. These
 * restrictions intentionally model "required object alias" semantics
 * rather than general-purpose address manipulation.
 *
 * The type preserves pointer-like ergonomics through dereference,
 * member access, raw-pointer interoperability, and covariant conversion
 * between compatible pointee types while enforcing a permanent engaged
 * invariant.
 *
 * Construction and rebinding from raw or pointer-like sources perform
 * runtime null validation and throw `std::invalid_argument` when a null
 * value is supplied.
 *
 * `required_ptr` does not own the referenced object and performs no
 * lifetime management. Users are responsible for ensuring the referenced
 * object outlives all aliases.
 *
 * Ordering comparisons and pointer arithmetic are intentionally deleted
 * to discourage misuse as an iterator or contiguous traversal type.
 *
 * `required_ptr<void>` is supported through the primary template to
 * enable type-erased non-null aliasing without introducing a dedicated
 * specialization.
 *
 * @todo Future Development: Use `= delete("reason")` instead of the C-style comments once the C++26 feature becomes available.
 */

//Module partition interface unit
export module base.vocab.ptr:required_ptr;

import std;

import base.meta.traits;
import base.meta.concepts;

import :core;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a required object alias.
     *
     * @tparam T The pointed-to type (must not be a reference type per "C++ standard [dcl.ptr]" nor a function pointer).
     *
     * @details `required_ptr` models a non-owning, non-nullable, non-arithmetic, void-permitting
     * pointer abstraction.
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `required_ptr` always refers to a valid object; there is no null/disengaged representation. Attempts to construct/assign from another pointer throw on null.
     * @remark This type does not own the referenced object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the pointer without affecting the lifetime of the underlying object.
     * @note Construction or assignment from `nullptr` is ill-formed.
     * @note Construction or assignment from raw pointer values performs runtime validation and throws `std::invalid_argument` on null. All operations provide the Strong Exception Safety Guarantee.
     * @note Non-arithmetic Pointer: Pointer arithmetic and ordering comparisons are intentionally disabled to prevent misuse as an iterator.
     * @note `required_ptr<void>` reuses the primary template rather than introducing a specialization. Placeholder reference aliases based on `std::monostate` are used solely to keep deleted overload declarations well-formed.
     * @remark Explicit equality comparison overloads are provided only where built-in pointer comparison cannot be reached through the implicit raw-pointer conversion operator alone.
     *
     * @warning The referenced object MUST outlive the `required_ptr`. Violating this results in undefined behavior.
     *
     * @see `alias_ptr` for nullable aliasing, `dependency_ptr` for dependency injection structural non-nullability, `cursor_ptr` for non-null iteration/traversal, `std::unique_ptr` and `std::shared_ptr` for ownership.
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
        using base_type = required_ptr::ptr_core;
    public:
        using base_type::base_type;
        using base_type::operator=;
    };

    /**
     * @brief Deduction guide for `required_ptr`.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     */
    template<typename T>
    required_ptr(T&) -> required_ptr<T>;

    /**
     * @brief Deduction guide for `required_ptr`.
     *
     * @tparam T The type of the pointee.
     *
     * @remark Deduces `T` from the pointee, preserving cv-qualification.
     */
    template<typename T>
    required_ptr(T*) -> required_ptr<T>;
} //namespace base::vocab::inline ptr
