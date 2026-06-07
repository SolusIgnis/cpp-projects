// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-policies.cppm
 * @version 0.6.0
 * @date June 6, 2026
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
 * @brief Pointer vocabulary policy infrastructure.
 *
 * @details Defines the policy vocabulary used by `ptr_core` to synthesize
 * pointer behavior at compile time as well as the policy validation and
 * compile-time querying facilities used throughout the pointer vocabulary
 * implementation.
 *
 * Policy groups:
 * - `traversal`
 *   - `arithmetic`
 *   - `rebinding`
 * - `reference_binding`
 *   - `allowed`
 *   - `forbidden`
 * - `pointer_binding`
 *   - `allowed`
 *   - `forbidden`
 * - `nullability`
 *   - `nullable`
 *   - `always_engaged`
 *
 * A valid `PtrPolicyList` selects exactly one policy from each
 * group. This partition also provides 
 */

//Module partition interface unit
export module base.vocab.ptr:policies;

import std;

import base.meta.sequences;

namespace base::vocab::inline ptr::ptr_policies {
    namespace traversal {
        ///@brief Policy-group marker for `traversal` policies.
        struct group;

        /**
         * @brief Enables arithmetic traversal semantics.
         *
         * @details Pointers using this policy allow ordered comparisons
         * and pointer arithmetic, and they satisfy the requirements of
         * contiguous iterators.
         */
        struct arithmetic {
            using policy_group = group;
        }; //struct arithmetic

        /**
         * @brief Enables rebinding-only traversal semantics.
         *
         * @details Pointers using this policy allow data structure
         * traversal via rebinding to different addresses (i.e., pointer
         * chasing) but do not support pointer arithmetic or address
         * ordering operations. These pointers are not iterators.
         */
        struct rebinding {
            using policy_group = group;
        }; //struct rebinding
    } //namespace traversal

    namespace reference_binding {
        ///@brief Policy-group marker for `reference_binding` policies.
        struct group;

        /**
         * @brief Allows binding (construction) and rebinding (assignment) from references.
         *
         * @details Pointers using this policy may be initialized or
         * rebound directly from lvalue references. Binding from
         * rvalue references (i.e. temporaries) remains prohibited to
         * reduce opportunities for dangling pointers.
         */
        struct allowed {
            using policy_group = group;
        }; //struct allowed

        /**
         * @brief Forbids binding (construction) and rebinding (assignment) from references.
         *
         * @details Pointers using this policy may not be initialized
         * or rebound from references and must instead support obtaining
         * their addresses through other mechanisms.
         */
        struct forbidden {
            using policy_group = group;
        }; //struct forbidden
    } //namespace reference_binding

    namespace pointer_binding {
        ///@brief Policy-group marker for `pointer_binding` policies.
        struct group;

        /**
         * @brief Allows binding (construction) and rebinding (assignment) from pointers.
         *
         * @details Pointers using this policy may be initialized
         * or rebound from raw pointers and compatible pointer-like
         * types exposing a suitable `get()` interface.
         */
        struct allowed {
            using policy_group = group;
        }; //struct allowed

        /**
         * @brief Forbids binding (construction) and rebinding (assignment) from pointers.
         *
         * @details Pointers using this policy reject raw pointer
         * and pointer-like inputs, allowing alternative binding
         * mechanisms to enforce stronger initialization invariants.
         */
        struct forbidden {
            using policy_group = group;
        }; //struct forbidden
    } //namespace pointer_binding

    namespace nullability {
        ///@brief Policy-group marker for `nullability` policies.
        struct group;

        /**
         * @brief Enables null pointer states.
         *
         * @details Pointers using this policy may be default
         * constructed, assigned from `nullptr`, released, and
         * tested for engagement through contextual conversion
         * to `bool`.
         */
        struct nullable {
            using policy_group = group;
        }; //struct nullable

        /**
         * @brief Enforces an always-engaged (i.e. non-null) invariant.
         *
         * @details Pointers using this policy cannot be default
         * constructed or assigned from `nullptr`. All instances
         * are required to remain bound to a valid address for
         * their entire lifetime.
         */
        struct always_engaged {
            using policy_group = group;
        }; //struct always_engaged
    } //namespace nullability

    /**
     * @brief Alias for a `type_list` metaprogramming container.
     * @tparam Elements The elements of the list.
     * @see `base::meta::sequences::type_list`
     * @internal
     */
    template<typename... Elements>
    using type_list = base::meta::sequences::type_list<Elements...>;

    /**
     * @brief Alias concept using the `TypeSequence` concept from `base.meta.sequences`.
     * @tparam T The candidate type.
     * @see `base::meta::sequences::TypeSequence`
     * @internal
     */
    template<typename T>
    concept TypeSequence = base::meta::sequences::TypeSequence<T>;

    ///@brief The canonical list of policy group tags defined in namespaces within `ptr_policies`.
    using policy_groups = type_list<traversal::group, reference_binding::group, pointer_binding::group, nullability::group>;

    /**
     * @brief Identifies policy-group marker types.
     *
     * @tparam T The candidate type.
     *
     * @details Satisfied when `T` is one of the policy-group tags
     * defining the mutually exclusive categories for pointer policies.
     *
     * @see `policy_groups`
     * @internal
     */
    template<typename T>
    concept PtrPolicyGroup = base::meta::sequences::contains_type_v<policy_groups, T>;

    /**
     * @brief Metafunction producing a unary type predicate that compares its argument's policy group against the expected group bound here.
     *
     * @tparam ExpectedPolicyGroup The `PtrPolicyGroup` to bind into the `predicate` template for comparison.
     */
    template<PtrPolicyGroup ExpectedPolicyGroup>
    struct in_policy_group {
        template<typename T>
            requires requires { typename T::policy_group; }
        struct predicate : std::bool_constant<std::same_as<typename T::policy_group, ExpectedPolicyGroup>> {};
    };

    
    template<PtrPolicyGroup Group, TypeSequence PolicyList>
    inline constexpr bool exactly_one_policy_v =
        base::meta::sequences::exactly_one_type_if_v<PolicyList, in_policy_group<Group>::template predicate>;

    template<TypeSequence GroupList, TypeSequence PolicyList>
    struct valid_policy_list;

    template<typename... Groups, TypeSequence PolicyList>
    struct valid_policy_list<type_list<Groups...>, PolicyList>
        : std::bool_constant<(... && exactly_one_policy_v<Groups, PolicyList>)> {};

    template<typename T>
    concept PtrPolicyList = TypeSequence<T> && valid_policy_list<policy_groups, T>::value;

    template<PtrPolicyList Policies, PtrPolicyGroup Group>
    using group_policy_t = base::meta::sequences::find_type_if_t<Policies, in_policy_group<Group>::template predicate>;

    template<PtrPolicyList Policies>
    inline constexpr bool arithmetic_traversal_v =
        std::same_as<ptr_policies::group_policy_t<Policies, traversal::group>, traversal::arithmetic>;

    template<PtrPolicyList Policies>
    inline constexpr bool rebinding_traversal_v =
        std::same_as<ptr_policies::group_policy_t<Policies, traversal::group>, traversal::rebinding>;

    template<PtrPolicyList Policies>
    inline constexpr bool allowed_reference_binding_v =
        std::same_as<ptr_policies::group_policy_t<Policies, reference_binding::group>, reference_binding::allowed>;

    template<PtrPolicyList Policies>
    inline constexpr bool forbidden_reference_binding_v =
        std::same_as<ptr_policies::group_policy_t<Policies, reference_binding::group>, reference_binding::forbidden>;

    template<PtrPolicyList Policies>
    inline constexpr bool allowed_pointer_binding_v =
        std::same_as<ptr_policies::group_policy_t<Policies, pointer_binding::group>, pointer_binding::allowed>;

    template<PtrPolicyList Policies>
    inline constexpr bool forbidden_pointer_binding_v =
        std::same_as<ptr_policies::group_policy_t<Policies, pointer_binding::group>, pointer_binding::forbidden>;

    template<PtrPolicyList Policies>
    inline constexpr bool nullable_nullability_v =
        std::same_as<ptr_policies::group_policy_t<Policies, nullability::group>, nullability::nullable>;

    template<PtrPolicyList Policies>
    inline constexpr bool always_engaged_nullability_v =
        std::same_as<ptr_policies::group_policy_t<Policies, nullability::group>, nullability::always_engaged>;
} //namespace base::vocab::inline ptr::ptr_policies
