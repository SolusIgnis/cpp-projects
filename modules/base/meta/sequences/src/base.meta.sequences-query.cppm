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
 *  ├── contains_v
 *  ├── count_v
 *  ├── equal_v
 *  ├── is_unique_v
 *  ├── exactly_one_of_v
 *  ├── index_of_v
 *  ├── count_if_v
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
    template<auto LHS, auto RHS>
    inline constexpr bool value_equivalent_v =
        std::same_as<
            std::remove_cvref_t<decltype(LHS)>,
            std::remove_cvref_t<decltype(RHS)>
        > && (LHS == RHS);
} //namespace base::meta::sequences

namespace base::meta::sequences {
    template<TypeSequence Seq, typename Query>
    struct contains_type;

    template<typename... Types, typename Query>
    struct contains_type<type_list<Types...>, Query>
        : std::bool_constant<
            (std::same_as<Query, Types> || ...)
        > {};

    template<ValueSequence Seq, auto Query>
    struct contains_value;

    template<auto... Values, auto Query>
    struct contains_value<value_list<Values...>, Query>
        : std::bool_constant<
            (value_equivalent_v<Query, Values> || ...)
        > {};

    template<typename T, T... Values, T Query>
    struct contains_value<uniform_value_list<T, Values...>, Query>
        : std::bool_constant<
            ((Query == Values) || ...)
        > {};

    export template<TypeSequence Seq, typename Query>
    inline constexpr bool contains_v =
        contains_type<std::remove_cvref_t<Seq>, Query>::value;

    export template<ValueSequence Seq, auto Query>
    inline constexpr bool contains_v<
        Seq,
        decltype(Query) Query>
    > =
        contains_value<
            std::remove_cvref_t<Seq>,
            decltype(Query) Query
        >::value;
} //namespace base::meta::sequences
