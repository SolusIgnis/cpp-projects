// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-metadata.cppm
 * @version 0.6.0
 * @date May 10, 2026
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
 * @brief Metadata for vocabulary pointer types.
 */

//Module partition interface unit
export module base.vocab.ptr:metadata;

import std;

namespace base::vocab::inline ptr {
    template<typename Pointee>
    struct pointer_metadata {
    private:
        struct void_reference;
    public:
        /**
         * @typedef element_type
         * @brief The stored element type.
         */
        using element_type = Pointee;

        /**
         * @typedef value_type
         * @brief The unqualified element type (`std::remove_cv_t<Pointee>`).
         */
        using value_type = std::remove_cv_t<Pointee>;

        /**
         * @typedef pointer
         * @brief The raw pointer type of the stored address (`Pointee*`).
         */
        using pointer = std::add_pointer_t<Pointee>;

        /**
         * @typedef reference
         * @brief The reference type (`Pointee&`).
         * @remark When `Pointee` is `void`, uses `void_reference&` because `void` as a function parameter is ill-formed.
         */
        using reference =
            std::conditional_t<std::is_void_v<Pointee>, std::add_lvalue_reference_t<void_reference>, std::add_lvalue_reference_t<Pointee>>;

        /**
         * @typedef rvalue_reference
         * @brief The rvalue reference type (`Pointee&&`).
         *
         * @remark When `Pointee` is `void`, uses `void_reference&&` because `void` as a function parameter is ill-formed.
         * @note Used only for deletion of invalid overloads to prevent binding to temporaries.
         */
        using rvalue_reference =
            std::conditional_t<std::is_void_v<Pointee>, std::add_rvalue_reference_t<void_reference>, std::add_rvalue_reference_t<Pointee>>;

        /**
         * @typedef difference_type
         * @brief Pointer difference type (`std::ptrdiff_t`).
         *
         * @note Provided to model pointer interface even when arithmetic is disabled.
         */
        using difference_type = std::ptrdiff_t;
    }; //struct pointer_metadata
} //namespace base::vocab::inline ptr
