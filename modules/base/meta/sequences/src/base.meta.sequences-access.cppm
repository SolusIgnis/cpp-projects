// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.sequences-access.cppm
 * @version 0.0.1
 * @date May 20, 2026
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
 * @brief Structural access utilities for compile-time sequences.
 * @details
 * :access
 * ​ ├── size_v
 * ​ ├── empty_v
 * ​ ├── EmptySequence
 * ​ ├── NonEmptySequence
 * ​ ├── front_t
 * ​ ├── front_v
​​​ *  ├── find_type_if_t
​ *  ├── find_value_if_v
 *  ├── at_t
 *  ├── at_v
​ *  ├── back_t
 * ​ └── back_v
 */

//Module partition interface unit
export module base.meta.sequences:access;

import std;

import :core;

namespace base::meta::sequences {
    template<Sequence Seq>
    struct sequence_size;

    template<typename... Types>
    struct sequence_size<type_list<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

    template<auto... Values>
    struct sequence_size<value_list<Values...>> : std::integral_constant<std::size_t, sizeof...(Values)> {};

    template<typename T, T... Values>
    struct sequence_size<uniform_value_list<T, Values...>> : std::integral_constant<std::size_t, sizeof...(Values)> {};

    export template<Sequence Seq>
    inline constexpr std::size_t size_v = sequence_size<std::remove_cvref_t<Seq>>::value;
} //namespace base::meta::sequences

export namespace base::meta::sequences {
    template<Sequence Seq>
    inline constexpr bool empty_v = (size_v<Seq> == 0);
    
    template<typename T>
    concept EmptySequence = Sequence<T> && empty_v<T>;
    
    template<typename T>
    concept NonEmptySequence = Sequence<T> && !EmptySequence<T>;
} //namespace base::meta::sequences

namespace base::meta::sequences {
    template<NonEmptySequence Seq>
    struct front;

    template<typename Front, typename... Rest>
    struct front<type_list<Front, Rest...>> {
        using type = Front;
    };

    template<auto Front, auto... Rest>
    struct front<value_list<Front, Rest...>> {
        static constexpr auto value = Front;
    };

    template<typename T, T Front, T... Rest>
    struct front<uniform_value_list<T, Front, Rest...>> {
        static constexpr T value = Front;
    };

    export template<TypeSequence Seq>
        requires NonEmptySequence<Seq>
    using front_t = typename front<std::remove_cvref_t<Seq>>::type;
    
    export template<ValueSequence Seq>
        requires NonEmptySequence<Seq>
    inline constexpr auto front_v = front<std::remove_cvref_t<Seq>>::value;
} //namespace base::meta::sequences

namespace base::meta::sequences {
    /**
     * @brief Finds the first type satisfying a unary type predicate.
     *
     * @tparam Seq A type sequence.
     * @tparam UnaryTypePredicate A unary type predicate.
     */
    template<TypeSequence Seq,
             template<typename> typename UnaryTypePredicate>
    struct find_type_if;

    /**
     * @brief Base case for empty type sequences.
     */
    template<template<typename> typename UnaryTypePredicate>
    struct find_type_if<
        type_list<>,
        UnaryTypePredicate
    > {
        static_assert(false, "find_if failure: result not found; use try_find_if for optional results");
    };

    /**
     * @brief Recursive short-circuit search over type sequences.
     */
    template<
        typename T,
        typename... Rest,
        template<typename> typename UnaryTypePredicate
    >
    struct find_type_if<
        type_list<T, Rest...>,
        UnaryTypePredicate
    > {
        using type = std::conditional_t<
            UnaryTypePredicate<T>::value,
            T,
            typename find_type_if<
                type_list<Rest...>,
                UnaryTypePredicate
            >::type
        >;
    };

    /**
     * @brief Finds the first value satisfying a unary value predicate.
     *
     * @tparam Seq A value sequence.
     * @tparam UnaryValuePredicate A unary value predicate.
     */
    template<ValueSequence Seq,
             template<auto> typename UnaryValuePredicate>
    struct find_value_if;

    /**
     * @brief Base case for empty heterogeneous value sequences.
     */
    template<template<auto> typename UnaryValuePredicate>
    struct find_value_if<
        value_list<>,
        UnaryValuePredicate
    > {
        static_assert(false, "find_if failure: result not found; use try_find_if for optional results");
    };

    /**
     * @brief Recursive short-circuit search over heterogeneous values.
     */
    template<
        auto Element,
        auto... Rest,
        template<auto> typename UnaryValuePredicate
    >
    struct find_value_if<
        value_list<Element, Rest...>,
        UnaryValuePredicate
    > {
        static constexpr auto value = (
            UnaryValuePredicate<Element>::value ?
            Element :
            find_value_if<
                value_list<Rest...>,
                UnaryValuePredicate
            >::value
        );
    };

    /**
     * @brief Base case for empty uniform value sequences.
     */
    template<
        typename T,
        template<auto> typename UnaryValuePredicate
    >
    struct find_value_if<
        uniform_value_list<T>,
        UnaryValuePredicate
    > {
        static_assert(false, "find_if failure: result not found; use try_find_if for optional results");
    };

    /**
     * @brief Recursive short-circuit search over uniform values.
     */
    template<
        typename T,
        T Element,
        T... Rest,
        template<auto> typename UnaryValuePredicate
    >
    struct find_value_if<
        uniform_value_list<T, Element, Rest...>,
        UnaryValuePredicate
    > {
        static constexpr T value = (
            UnaryValuePredicate<Element>::value ?
            Element :
            find_value_if<
                uniform_value_list<T, Rest...>,
                UnaryValuePredicate
            >::value
        );
    };

    /**
     * @brief Alias for the first type satisfying a predicate.
     */
    export template<
        TypeSequence Seq,
        template<typename> typename UnaryTypePredicate
    >
        requires NonEmptySequence<Seq>
    using find_type_if_t =
        typename find_type_if<
            std::remove_cvref_t<Seq>,
            UnaryTypePredicate
        >::type;

    /**
     * @brief Alias for the first value satisfying a predicate.
     */
    export template<
        ValueSequence Seq,
        template<auto> typename UnaryValuePredicate
    >
        requires NonEmptySequence<Seq>
    inline constexpr auto find_value_if_v =
        find_value_if<
            std::remove_cvref_t<Seq>,
            UnaryValuePredicate
        >::value;
} // namespace base::meta::sequences
