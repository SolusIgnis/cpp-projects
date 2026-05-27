// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-policies.cppm
 * @version 0.6.0
 * @date May 18, 2026
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
 */

//Module partition interface unit
export module base.vocab.ptr:policies;

import std;

import base.meta.sequences;
import base.meta.traits;
import base.meta.concepts;

namespace base::vocab::inline ptr::ptr_policies {
    namespace traversal {
        struct group;

        struct arithmetic {
            using policy_group = group;
        }; //class arithmetic

        struct rebinding {
            using policy_group = group;
        }; //class rebinding
    } //namespace traversal

    namespace reference_binding {
        struct group;

        struct allowed {
            using policy_group = group;
        }; //class allowed

        struct forbidden {
            using policy_group = group;
        }; //class forbidden
    } //namespace reference_binding

    namespace pointer_binding {
        struct group;

        struct allowed {
            using policy_group = group;
        }; //class allowed

        struct forbidden {
            using policy_group = group;
        }; //class forbidden
    } //namespace pointer_binding

    namespace nullability {
        struct group;

        struct yes {
            using policy_group = group;
        }; //class yes

        struct no {
            using policy_group = group;
        }; //class no
    } //namespace nullability

    template<typename... Elements>
    using type_list = base::meta::sequences::type_list<Elements...>;

    template<typename T>
    concept TypeSequence = base::meta::sequences::TypeSequence<T>;

    using policy_groups = type_list<traversal::group, reference_binding::group, pointer_binding::group, nullability::group>;

    template<typename T>
    concept PtrPolicyGroup = base::meta::sequences::contains_type_v<policy_groups, T>;

    template<PtrPolicyGroup ExpectedPolicyGroup>
    struct in_policy_group {
        template<typename T>
            requires requires { typename T::policy_group; }
        struct predicate : std::bool_constant<std::same_as<typename T::policy_group, ExpectedPolicyGroup>> {};
    };

    template<PtrPolicyGroup Group, TypeSequence PolicyList>
    inline constexpr bool exactly_one_policy_v =
        (base::meta::sequences::count_type_if_v<PolicyList, in_policy_group<Group>::template predicate> == 1);

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
    inline constexpr bool nullable_v =
        std::same_as<ptr_policies::group_policy_t<Policies, nullability::group>, nullability::yes>;

    template<PtrPolicyList Policies>
    inline constexpr bool nonnullable_v =
        std::same_as<ptr_policies::group_policy_t<Policies, nullability::group>, nullability::no>;
} //namespace base::vocab::inline ptr::ptr_policies

namespace base::vocab::inline ptr {
    static_assert(ptr_policies::in_policy_group<ptr_policies::nullability::group>::template predicate<ptr_policies::nullability::no>::value,  "in_policy_group bug");
    using test_policy_set = ptr_policies::type_list<
            ptr_policies::nullability::no,
            ptr_policies::pointer_binding::forbidden,
            ptr_policies::reference_binding::allowed,
            ptr_policies::traversal::rebinding
        >;
    static_assert(ptr_policies::PtrPolicyList<test_policy_set>, "policy set is invalid");
    
    template<typename T>
    struct nullability_predicate :
        ptr_policies::in_policy_group<
            ptr_policies::nullability::group
        >::template predicate<T>
    {};
    using extracted =
        base::meta::sequences::find_type_if_t<
            test_policy_set,
            nullability_predicate
        >;
    static_assert(std::same_as<extracted, ptr_policies::nullability::no>, "same_as nullability::no failed");
    static_assert(ptr_policies::nonnullable_v<test_policy_set>, "nonnullable_v failed");
}
