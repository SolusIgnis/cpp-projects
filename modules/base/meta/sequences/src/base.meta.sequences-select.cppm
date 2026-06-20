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
 *  ├── try_find_value_if_t
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
     * @brief Finds the first type satisfying a unary type predicate.
     *
     * @tparam Seq A type sequence.
     * @tparam UnaryTypePredicate A unary type predicate.
     */
    template<TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    struct try_find_type_if;

    template<bool, typename T, TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    struct try_find_type_if_impl;

    template<typename T, TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    struct try_find_type_if_impl<true, T, Seq, UnaryTypePredicate> {
        using type = type_list<T>;
    };

    template<typename T, TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    struct try_find_type_if_impl<false, T, Seq, UnaryTypePredicate> {
        using type = typename try_find_type_if<Seq, UnaryTypePredicate>::type;
    };

    /**
     * @brief Base case for empty type sequences.
     */
    template<template<typename> typename UnaryTypePredicate>
    struct try_find_type_if<type_list<>, UnaryTypePredicate> {
        using type = type_list<>;
    };

    /**
     * @brief Recursive short-circuit search over type sequences.
     */
    template<typename T, typename... Rest, template<typename> typename UnaryTypePredicate>
    struct try_find_type_if<type_list<T, Rest...>, UnaryTypePredicate>
        : try_find_type_if_impl<UnaryTypePredicate<T>::value, T, type_list<Rest...>, UnaryTypePredicate> {};

    /**
     * @brief Finds the first value satisfying a unary value predicate.
     *
     * @tparam Seq A value sequence.
     * @tparam UnaryValuePredicate A unary value predicate.
     */
    template<ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    struct try_find_value_if;

    template<bool, auto Element, ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    struct try_find_value_if_impl;

    template<auto Element, ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    struct try_find_value_if_impl<true, Element, Seq, UnaryValuePredicate> {
        using type = value_list<Element>;
    };

    template<auto Element, ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    struct try_find_value_if_impl<false, Element, Seq, UnaryValuePredicate> {
        using type = typename try_find_value_if<Seq, UnaryValuePredicate>::type;
    };

    /**
     * @brief Base case for empty heterogeneous value sequences.
     */
    template<template<auto> typename UnaryValuePredicate>
    struct try_find_value_if<value_list<>, UnaryValuePredicate> {
        using type = value_list<>;
    };

    /**
     * @brief Recursive short-circuit search over heterogeneous values.
     */
    template<auto Element, auto... Rest, template<auto> typename UnaryValuePredicate>
    struct try_find_value_if<value_list<Element, Rest...>, UnaryValuePredicate>
        : try_find_value_if_impl<UnaryValuePredicate<Element>::value, Element, value_list<Rest...>, UnaryValuePredicate> {};

    template<typename T, bool, T Element, ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    struct try_find_uniform_value_if_impl;

    template<typename T, T Element, ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    struct try_find_uniform_value_if_impl<T, true, Element, Seq, UnaryValuePredicate> {
        using type = uniform_value_list<T, Element>;
    };

    template<typename T, T Element, ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    struct try_find_uniform_value_if_impl<T, false, Element, Seq, UnaryValuePredicate> {
        using type = typename try_find_value_if<Seq, UnaryValuePredicate>::type;
    };

    /**
     * @brief Base case for empty uniform value sequences.
     */
    template<typename T, template<auto> typename UnaryValuePredicate>
    struct try_find_value_if<uniform_value_list<T>, UnaryValuePredicate> {
        using type = uniform_value_list<T>;
    };

    /**
     * @brief Recursive short-circuit search over uniform values.
     */
    template<typename T, T Element, T... Rest, template<auto> typename UnaryValuePredicate>
    struct try_find_value_if<uniform_value_list<T, Element, Rest...>, UnaryValuePredicate>
        : try_find_uniform_value_if_impl<T, UnaryValuePredicate<Element>::value, Element, uniform_value_list<T, Rest...>, UnaryValuePredicate> {};

    /**
     * @brief Alias for the first type satisfying a predicate.
     */
    export template<TypeSequence Seq, template<typename> typename UnaryTypePredicate>
    using try_find_type_if_t = typename try_find_type_if<std::remove_cvref_t<Seq>, UnaryTypePredicate>::type;

    /**
     * @brief Alias for the first value satisfying a predicate.
     */
    export template<ValueSequence Seq, template<auto> typename UnaryValuePredicate>
    using try_find_value_if_t = typename try_find_value_if<std::remove_cvref_t<Seq>, UnaryValuePredicate>::type;
} // namespace base::meta::sequences
