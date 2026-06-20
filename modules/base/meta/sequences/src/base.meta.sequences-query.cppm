// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.sequences-query.cppm
 * @version 0.0.1
 * @date May 21, 2026
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
 * @brief Query utilities for compile-time sequences.
 * @details
 * :query
 *  ├── value_equivalent_v
 * ​ ├── uniform_equivalent_v
 *  ├── contains_v
 *  ├── contains_type_v
 *  ├── contains_value_v
 *  ├── count_if_v
 *  ├── count_type_if_v
 *  ├── count_value_if_v
 *  ├── exactly_one_type_if_v
 *  ├── exactly_one_value_if_v
 *  ├── count_v
 *  ├── count_type_v
 *  ├── count_value_v
 *  ├── exactly_one_of_v
 *  ├── exactly_one_type_of_v
 *  ├── exactly_one_value_of_v
 *  ├── equal_v
 *  ├── is_unique_v
 *  ├── index_of_v
 *  ├── index_of_type_v
 *  ├── index_of_value_v
 *  ├── any_of_v
 *  ├── all_of_v
 *  └── none_of_v
 */

//Module partition interface unit
export module base.meta.sequences:query;

import std;

import :core;
import :access;

namespace base::meta::sequences {
    /**
     * @internal
     * @brief Equivalence comparison requires same cv-ref normalized type and value equality.
     */
    template<auto LHS, auto RHS>
    inline constexpr bool value_equivalent_v =
        std::same_as<std::remove_cvref_t<decltype(LHS)>, std::remove_cvref_t<decltype(RHS)>> && (LHS == RHS);

    /**
     * @internal
     * @brief Equivalence comparison with uniform types requires value equality.
     * @pre std::same_as<std::remove_cvref_t<decltype(LHS)>, std::remove_cvref_t<decltype(RHS)>>
     * @note This is designed for `uniform_value_list` element comparisons where the elements are already guaranteed to be the same type.
     */
    template<auto LHS, auto RHS>
    inline constexpr bool uniform_equivalent_v = (LHS == RHS);
} //namespace base::meta::sequences

namespace base::meta::sequences {
    template<TypeSequence Seq, typename Query>
    struct contains_type;

    template<typename... Values, typename Query>
    struct contains_type<type_list<Values...>, Query> : std::bool_constant<(std::same_as<Query, Values> || ...)> {};

    template<ValueSequence Seq, auto Query>
    struct contains_value;

    template<auto... Values, auto Query>
    struct contains_value<value_list<Values...>, Query> : std::bool_constant<(value_equivalent_v<Query, Values> || ...)> {};

    template<typename T, T... Values, T Query>
    struct contains_value<uniform_value_list<T, Values...>, Query>
        : std::bool_constant<(uniform_equivalent_v<Query, Values> || ...)> {};

    export template<TypeSequence Seq, typename Query>
    inline constexpr bool contains_type_v = contains_type<std::remove_cvref_t<Seq>, Query>::value;

    export template<ValueSequence Seq, auto Query>
    inline constexpr bool contains_value_v = contains_value<std::remove_cvref_t<Seq>, Query>::value;
} //namespace base::meta::sequences

namespace base::meta::sequences {
    template<TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    struct count_type_if;

    template<typename... Elements, template<typename> typename UnaryTypePredicate>
    struct count_type_if<type_list<Elements...>, UnaryTypePredicate>
        : std::integral_constant<std::size_t, (0 + ... + (UnaryTypePredicate<Elements>::value ? 1 : 0))> {};

    template<ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    struct count_value_if;

    template<auto... Elements, template<auto> typename UnaryValuePredicate>
    struct count_value_if<value_list<Elements...>, UnaryValuePredicate>
        : std::integral_constant<std::size_t, (0 + ... + (UnaryValuePredicate<Elements>::value ? 1 : 0))> {};

    template<typename T, T... Elements, template<auto> typename UnaryValuePredicate>
    struct count_value_if<uniform_value_list<T, Elements...>, UnaryValuePredicate>
        : std::integral_constant<std::size_t, (0 + ... + (UnaryValuePredicate<Elements>::value ? 1 : 0))> {};

    export template<TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    inline constexpr std::size_t count_type_if_v = count_type_if<std::remove_cvref_t<Seq>, UnaryTypePredicate>::value;

    export template<ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    inline constexpr std::size_t count_value_if_v = count_value_if<std::remove_cvref_t<Seq>, UnaryValuePredicate>::value;

    export template<TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    inline constexpr bool exactly_one_type_if_v = (count_type_if_v<Seq, UnaryTypePredicate> == 1);

    export template<ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    inline constexpr bool exactly_one_value_if_v = (count_value_if_v<Seq, UnaryValuePredicate> == 1);
} //namespace base::meta::sequences

namespace base::meta::sequences {
    template<TypeSequence Seq, typename Query>
    struct count_type;

    template<typename... Elements, typename Query>
    struct count_type<type_list<Elements...>, Query>
        : std::integral_constant<std::size_t, (0 + ... + (std::same_as<Query, Elements> ? 1 : 0))> {};

    template<ValueSequence Seq, auto Query>
    struct count_value;

    template<auto... Elements, auto Query>
    struct count_value<value_list<Elements...>, Query>
        : std::integral_constant<std::size_t, (0 + ... + (value_equivalent_v<Query, Elements> ? 1 : 0))> {};

    template<typename T, T... Elements, T Query>
    struct count_value<uniform_value_list<T, Elements...>, Query>
        : std::integral_constant<std::size_t, (0 + ... + (uniform_equivalent_v<Query, Elements> ? 1 : 0))> {};

    export template<TypeSequence Seq, typename Query>
    inline constexpr std::size_t count_type_v = count_type<std::remove_cvref_t<Seq>, Query>::value;

    export template<ValueSequence Seq, auto Query>
    inline constexpr std::size_t count_value_v = count_value<std::remove_cvref_t<Seq>, Query>::value;

    export template<TypeSequence Seq, typename Query>
    inline constexpr bool exactly_one_type_of_v = (count_type_v<Seq, Query> == 1);

    export template<ValueSequence Seq, auto Query>
    inline constexpr bool exactly_one_value_of_v = (count_value_v<Seq, Query> == 1);
} //namespace base::meta::sequences

namespace base::meta::sequences {
    template<TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    struct any_of_type;

    template<typename... Elements, template<typename> typename UnaryTypePredicate>
    struct any_of_type<type_list<Elements...>, UnaryTypePredicate>
        : std::bool_constant<(false || ... || UnaryTypePredicate<Elements>::value)> {};

    template<ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    struct any_of_value;

    template<auto... Elements, template<auto> typename UnaryValuePredicate>
    struct any_of_value<value_list<Elements...>, UnaryValuePredicate>
        : std::bool_constant<(false || ... || UnaryValuePredicate<Elements>::value)> {};

    template<typename T, T... Elements, template<auto> typename UnaryValuePredicate>
    struct any_of_value<uniform_value_list<T, Elements...>, UnaryValuePredicate>
        : std::bool_constant<(false || ... || UnaryValuePredicate<Elements>::value)> {};

    template<TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    struct all_of_type;

    template<typename... Elements, template<typename> typename UnaryTypePredicate>
    struct all_of_type<type_list<Elements...>, UnaryTypePredicate>
        : std::bool_constant<(true && ... && UnaryTypePredicate<Elements>::value)> {};

    template<ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    struct all_of_value;

    template<auto... Elements, template<auto> typename UnaryValuePredicate>
    struct all_of_value<value_list<Elements...>, UnaryValuePredicate>
        : std::bool_constant<(true && ... && UnaryValuePredicate<Elements>::value)> {};

    template<typename T, T... Elements, template<auto> typename UnaryValuePredicate>
    struct all_of_value<uniform_value_list<T, Elements...>, UnaryValuePredicate>
        : std::bool_constant<(true && ... && UnaryValuePredicate<Elements>::value)> {};

    export template<TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    inline constexpr bool any_of_type_v = any_of_type<std::remove_cvref_t<Seq>, UnaryTypePredicate>::value;

    export template<ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    inline constexpr bool any_of_value_v = any_of_value<std::remove_cvref_t<Seq>, UnaryValuePredicate>::value;

    export template<TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    inline constexpr bool all_of_type_v = all_of_type<std::remove_cvref_t<Seq>, UnaryTypePredicate>::value;

    export template<ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    inline constexpr bool all_of_value_v = all_of_value<std::remove_cvref_t<Seq>, UnaryValuePredicate>::value;

    export template<TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    inline constexpr bool none_of_type_v = !any_of_type_v<Seq, UnaryTypePredicate>;

    export template<ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    inline constexpr bool none_of_value_v = !any_of_value_v<Seq, UnaryValuePredicate>;
} //namespace base::meta::sequences
