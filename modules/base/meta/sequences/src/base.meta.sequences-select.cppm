// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

/**
 * @file base.meta.sequences-select.cppm
 * @version 0.0.1
 * @date May 25, 2026
 *
 * @brief Sequence selection and extraction algorithms.
 *
 * @details
 * :select
 * ​ ├── filter_t
​ *  ├── remove_t
​ *  ├── try_find_type_if_t
 * ​ ├── extract_t
 * ​ ├── partition_t
 * ​ ├── drop_t
 * ​ └── take_t
 *
 * Provides selector-oriented metafunctions for compile-time sequence
 * traversal, extraction, and filtering operations.
 */

export module base.meta.sequences:select;

import std;

import :core;

namespace base::meta::sequences {
    /**
     * @brief Sentinel type indicating no matching element was found.
     */
    struct not_found {};

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
        using type = not_found;
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
    > {};

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
    > {};

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
    inline constexpr auto find_value_if_v =
        find_value_if<
            std::remove_cvref_t<Seq>,
            UnaryValuePredicate
        >::value;
} // namespace base::meta::sequences
