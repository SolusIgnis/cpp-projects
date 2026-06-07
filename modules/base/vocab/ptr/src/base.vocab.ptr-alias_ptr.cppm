// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-alias_ptr.cppm
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
 * @brief `alias_ptr`: A non-owning, nullable, non-arithmetic, void-permitting pointer type.
 *
 * @details
 *
 * `alias_ptr` is a vocabulary pointer type representing a non-owning,
 * nullable, object alias without traversal semantics.
 *
 * Unlike owning smart pointers, `alias_ptr` does not participate in
 * object lifetime management. Unlike iterators or built-in pointers,
 * it intentionally disables pointer arithmetic and ordering comparisons
 * so that it models object association rather than memory traversal.
 *
 * The type exists to provide an explicit semantic alternative to direct
 * use of built-in pointers in codebases where pointer intent is treated
 * as part of the type system. In particular, `alias_ptr` communicates:
 *
 * - nullable engagement,
 * - non-ownership,
 * - stable object association,
 * - and non-iterative usage.
 *
 * `alias_ptr` preserves interoperability with existing pointer-based APIs
 * through implicit conversion to its nested `pointer` type and through
 * `get()`, while still enabling APIs and generic code to distinguish
 * semantic object references from ownership-bearing or traversal-oriented
 * pointer abstractions.
 *
 * The interface intentionally rejects several categories of operations:
 * - binding to temporaries,
 * - implicit C-array decay,
 * - pointer arithmetic,
 * - and address ordering comparisons.
 *
 * These restrictions exist to reduce accidental misuse and to reinforce
 * the intended semantic role of the type as an object alias rather than
 * a generalized memory navigation primitive.
 *
 * `alias_ptr` forms the nullable foundation of the pointer vocabulary
 * hierarchy:
 *
 * - `alias_ptr`        : nullable object alias
 * - `required_ptr`     : non-null object alias
 * - `dependency_ptr`   : structurally non-null injected dependency
 * - `cursor_ptr`       : traversal-oriented non-null pointer
 *
 * This separation allows APIs to express pointer expectations directly
 * in their type signatures while remaining lightweight, zero-overhead,
 * and interoperable with existing pointer-based interfaces.
 *
 * @todo Future Development: Use `= delete("reason")` once the C++26 feature becomes available.
 */

//Module partition interface unit
export module base.vocab.ptr:alias_ptr;

import std;

import base.meta.traits;
import base.meta.concepts;

#ifndef LEGACY_POINTER_IMPLEMENTATION
import :core;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a potentially null object alias.
     *
     * @tparam T The pointed-to type (must not be a reference type per "C++ standard [dcl.ptr]" nor a function pointer).
     *
     * @details `alias_ptr` models a non-owning, nullable, non-arithmetic, void-permitting
     * pointer abstraction.
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @remark This type does not own the referenced object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the stored address without affecting pointee lifetime.
     * @note Non-arithmetic Pointer: Pointer arithmetic and ordering comparisons are intentionally disabled to prevent misuse as an iterator. @see cursor_ptr for traversal/iteration semantics.
     * @note `alias_ptr<void>` reuses the primary template rather than introducing a specialization. Placeholder reference aliases based on `std::monostate` are used solely to keep deleted overload declarations well-formed.
     * @remark Explicit equality comparison overloads are provided only where implicit conversion to the nested `pointer` type is insufficient to enable the desired comparison.
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
            ptr_policies::pointer_binding::allowed,
            ptr_policies::reference_binding::allowed,
            ptr_policies::traversal::rebinding
        >
    > {
    private:
        using base_type = alias_ptr::ptr_core;
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
#endif
