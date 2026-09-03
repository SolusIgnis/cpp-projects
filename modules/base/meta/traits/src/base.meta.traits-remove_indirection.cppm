// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.traits-remove_indirection.cppm
 * @version 0.0.4
 * @date April 27, 2026
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
 * @brief `remove_indirection_t`: A type transformation trait that removes one layer of indirection from a type.
 *
 * @details `remove_indirection_t` peels off one layer of indirection
 * from a type, including:
 *   - pointers (`T*`) with any cv-qualification.
 *   - lvalue references (`T&`).
 *   - rvalue references (`T&&`).
 *   - array extents (`T[]` and `T[N]`).
 *   - pointers-to-member (`T C::*`) with any cv-qualification.
 *
 * Conceptually, this transformation is equivalent to applying
 * `std::remove_pointer`, `std::remove_reference`, and `std::remove_extent`
 * with the additional rule that pointers-to-member (`T C::*`) are treated
 * analogously to pointers and removed in the same manner.
 */

//Module partition interface unit
export module base.meta.traits:remove_indirection;

import std;

namespace base::meta::traits::inline transformation {
    ///@internal Primary template: Identity transformation for non-indirect types.
    template<typename T>
    struct remove_indirection {
        using type = T;
    };

    ///@internal Pointer Specializations
    template<typename T>
    struct remove_indirection<T*> {
        using type = T;
    };

    template<typename T>
    struct remove_indirection<T* const> {
        using type = T;
    };

    template<typename T>
    struct remove_indirection<T* volatile> {
        using type = T;
    };

    template<typename T>
    struct remove_indirection<T* const volatile> {
        using type = T;
    };

    ///@internal Pointer-to-Member Specializations
    template<typename T, typename C>
    struct remove_indirection<T C::*> {
        using type = T;
    };

    template<typename T, typename C>
    struct remove_indirection<T C::* const> {
        using type = T;
    };

    template<typename T, typename C>
    struct remove_indirection<T C::* volatile> {
        using type = T;
    };

    template<typename T, typename C>
    struct remove_indirection<T C::* const volatile> {
        using type = T;
    };

    ///@internal Reference Specializations
    template<typename T>
    struct remove_indirection<T&> {
        using type = T;
    };

    template<typename T>
    struct remove_indirection<T&&> {
        using type = T;
    };

    ///@internal Array Specializations
    template<typename T, std::size_t N>
    //NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays): This type trait removes array extents.
    struct remove_indirection<T[N]> {
        using type = T;
    };

    template<typename T>
    //NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays): This type trait removes array extents.
    struct remove_indirection<T[]> {
        using type = T;
    };
} //namespace base::meta::traits::inline transformation

export namespace base::meta::traits::inline transformation {
    ///@brief Alias for `remove_indirection<T>::type`.
    template<typename T>
    using remove_indirection_t = remove_indirection<T>::type;
} //namespace base::meta::traits::inline transformation
