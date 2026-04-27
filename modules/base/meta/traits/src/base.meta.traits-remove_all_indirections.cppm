// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.traits-remove_all_indirections.cppm
 * @version 0.0.1
 * @date April 26, 2026
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
 * @brief `remove_all_indirections_t`: A type transformation trait that removes all layers of indirection yielding the core cv-qualified type.
 *
 * @details `remove_all_indirections_t` recursively peels off all layers of
 * indirection from a type, including pointers, lvalue references, rvalue
 * references, array extents, and pointers-to-member. Pointers and pointers-
 * to-member are removed regardless of cv-qualification on the pointer itself.
 * This yields the core cv-qualified type.
 */

//Module partition interface unit
export module base.meta.traits:remove_all_indirections;

import std;

namespace base::meta::traits::inline transformation {
    template<typename T>
    struct remove_all_indirections {
        using type = T;
    };

    template<typename T>
    struct remove_all_indirections<T*> : remove_all_indirections<T> {};

    template<typename T>
    struct remove_all_indirections<T* const> : remove_all_indirections<T> {};

    template<typename T>
    struct remove_all_indirections<T* volatile> : remove_all_indirections<T> {};

    template<typename T>
    struct remove_all_indirections<T* const volatile> : remove_all_indirections<T> {};

    template<typename T, typename C>
    struct remove_all_indirections<T C::*> : remove_all_indirections<T> {};

    template<typename T, typename C>
    struct remove_all_indirections<T C::* const> : remove_all_indirections<T> {};

    template<typename T, typename C>
    struct remove_all_indirections<T C::* volatile> : remove_all_indirections<T> {};

    template<typename T, typename C>
    struct remove_all_indirections<T C::* const volatile> : remove_all_indirections<T> {};

    template<typename T>
    struct remove_all_indirections<T&> : remove_all_indirections<T> {};

    template<typename T>
    struct remove_all_indirections<T&&> : remove_all_indirections<T> {};

    template<typename T, std::size_t N>
    struct remove_all_indirections<T[N]> : remove_all_indirections<T> {};

    template<typename T>
    struct remove_all_indirections<T[]> : remove_all_indirections<T> {};
} //namespace base::meta::traits::inline transformation

export namespace base::meta::traits::inline transformation {
    template<typename T>
    using remove_all_indirections_t = typename remove_all_indirections<T>::type;
} //namespace base::meta::traits::inline transformation
