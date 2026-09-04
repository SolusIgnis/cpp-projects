// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @module base.vocab.tagging
 * @file base.vocab.tagging.cppm
 * @version 1.0.0
 * @date September 3, 2026
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
 * @brief Primary module interface for generic semantic tagging.
 */

//Primary module interface unit
export module base.vocab.tagging;

import std;

export namespace base::vocab::inline tagging{
    /**
     * @brief Applies a transient semantic tag across an interface boundary.
     *
     * @tparam Tag The semantic tag type.
     * @tparam T The underlying value type being tagged. Must satisfy `std::move_constructible`.
     *
     * @details
     * `tagged_boundary` is a single-use prvalue wrapper designed to enforce site-of-call
     * type safety across function parameter boundaries without persisting wrapper overhead.
     * It constructs an underlying value of type `T` in-place and converts destructively to
     * `T` via rvalue conversion (`operator T() &&`).
     *
     * ## Transient Semantics
     * `tagged_boundary` explicitly deletes all copy and move special member functions.
     * It cannot be stored as a class member, held in a local variable for reuse, or
     * passed around beyond its immediate call-site expression. Guaranteed copy elision
     * ensures that passing a prvalue `tagged_boundary` to a function parameter constructs
     * the wrapper directly in the parameter's storage.
     *
     * @example @parblock
     * ## Example Usage
     * @code
     * class option_enablement_predicates {
     *     struct local_tag {};
     *     struct remote_tag {};
     *
     * public:
     *     using local_predicate  = base::vocab::tagged_boundary<local_tag, enable_predicate_type>;
     *     using remote_predicate = base::vocab::tagged_boundary<remote_tag, enable_predicate_type>;
     *
     * private:
     *     local_predicate local_pred_;
     *     remote_predicate remote_pred_;
     *
     * public:
     *     explicit option_enablement_predicates(
     *         local_predicate local_pred   = local_predicate{always_reject},
     *         remote_predicate remote_pred = remote_predicate{always_reject}
     *     ) : local_pred_(std::move(local_pred)),
     *         remote_pred_(std::move(remote_pred)) {}
     * };
     *
     * // Call site prevents accidental parameter swapping:
     * option_enablement_predicates predicates(
     *     option_enablement_predicates::local_predicate{my_local_fn},
     *     option_enablement_predicates::remote_predicate{my_remote_fn}
     * );
     * @endcode @endparblock
     */
    template<typename Tag, std::move_constructible T>
    class tagged_boundary {
        T value_;
    public:
        ///@brief Constructs the underlying value @p T in-place.
        template<typename... Args>
            requires (sizeof...(Args) != 1 || (!std::same_as<std::remove_cvref_t<Args>, tagged_boundary> && ...)) && std::constructible_from<T, Args...>
        constexpr explicit tagged_boundary(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : value_(std::forward<Args>(args)...) {}

        ///@brief Default destructor.
        ~tagged_boundary() = default;

        ///@brief Destructively extracts the underlying `T` value.
        [[nodiscard]] constexpr explicit(false) operator T() && noexcept(std::is_nothrow_move_constructible_v<T>)
        { return std::move(value_); }

        tagged_boundary(const tagged_boundary&)            = delete;
        tagged_boundary& operator=(const tagged_boundary&) = delete;
        tagged_boundary(tagged_boundary&&)                 = delete;
        tagged_boundary& operator=(tagged_boundary&&)      = delete;
    }; //class tagged_boundary
    /**
     * @fn constexpr explicit tagged_boundary::tagged_boundary(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
     *
     * @tparam Args Argument types forwarded to `T`'s constructor.
     * @param args Arguments to forward to `T`'s constructor.
     * @throw Anything thrown by `T`'s selected constructor.
     *
     * Forwards all arguments directly to the constructor of `T`.
     * Constrained to prevent hijacking by copy/move operations or self-referential initialization.
     */
    /**
     * @overload tagged_boundary::tagged_boundary(const tagged_boundary&) = delete
     *
     * @brief Deleted copy constructor to ensure immovable objects.
     */
    /**
     * @overload tagged_boundary::tagged_boundary(tagged_boundary&&) = delete
     *
     * @brief Deleted move constructor to ensure immovable objects.
     */
    /**
     * @fn tagged_boundary& tagged_boundary::operator=(const tagged_boundary&) = delete
     *
     * @brief Deleted copy assignment to ensure immovable objects.
     */
    /**
     * @overload tagged_boundary& tagged_boundary::operator=(tagged_boundary&&) = delete
     *
     * @brief Deleted move assignment to ensure immovable objects.
     */
    /**
     * @fn constexpr explicit(false) tagged_boundary::operator T() && noexcept(std::is_nothrow_move_constructible_v<T>)
     *
     * Implicitly converts an rvalue `tagged_boundary` into `T` via move construction.
     *
     * @return The underlying value `T` moved out of the boundary wrapper.
     * @throw Anything thrown by `T`'s move constructor.
     */
} //namespace base::vocab::inline tagging
