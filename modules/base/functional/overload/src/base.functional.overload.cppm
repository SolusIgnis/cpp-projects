// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @module base.functional.overload
 * @file base.functional.overload.cppm
 * @version 1.0.0
 * @date April 17, 2026
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
 * @brief Primary module interface for aggregating callable overload sets (the overload pattern).
 */

//Primary module interface unit
export module base.functional.overload;

export namespace base::functional {
    /**
     * @brief Aggregates multiple callable objects into a single overload set.
     *
     * `overload` is a variadic utility that constructs a single callable object
     * with an overload set composed from heterogeneous callables, most commonly
     * lambdas. By inheriting from multiple lambdas or other function objects and
     * bringing their call operators into the local scope, it allows for concise,
     * ad-hoc pattern matching. This "overload pattern" is frequently used in
     * conjunction with generic dispatch or visitation (via `std::visit`) of a
     * `std::variant`. Overload resolution follows standard rules across all
     * inherited function call operator members.
     *
     * @tparam Callables Callable types to be aggregated.
     *
     * @note `overload` produces a function object with a function call operator
     *       whose overload set is the union of the overload sets of its bases.
     *       For any given argument list, overload resolution must yield a unique
     *       best viable function within this union. If multiple candidates are
     *       viable and none is strictly better than the others, the call is ill-
     *       formed due to ambiguity.
     *
     * @example @parblock
     * ## Example Usage
     * @code
     * auto f = base::functional::overload{
     *     [](int i)  { return i + 1; },
     *     [](char c) { return c - ('a' - 'A'); }
     * };
     *
     * f('c'); // calls char overload
     * f(3);   // calls int overload
     * @endcode @endparblock
     *
     * @example @parblock
     * ## Example Usage (`std::visit`)
     * @code
     * std::variant<int, float, std::string> v = "Hello World";
     *
     * std::visit(overload {
     *     [](int i) { std::cout << "Integer: " << i << "\n"; },
     *     [](float f) { std::cout << "Float: " << f << "\n"; },
     *     [](const std::string& s) { std::cout << "String: " << s << "\n"; }
     * }, v);
     * @endcode @endparblock
     */
    template<typename... Callables>
    struct overload : Callables... {
        ///@brief Introduces the call operators of all base classes into the current scope.
        using Callables::operator()...;
    };

    ///@brief Deduction guide to enable CTAD (Class Template Argument Deduction).
    template<typename... Callables>
    overload(Callables...) -> overload<Callables...>;
} //namespace base::functional
