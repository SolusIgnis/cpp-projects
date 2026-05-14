// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-nullability_policies.cppm
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
 * @brief Policies governing pointer nullability.
 */

//Module partition interface unit
export module base.vocab.ptr:nullability_policies;

import std;

import base.meta.traits;
import base.meta.concepts;

namespace base::vocab::inline ptr::ptr_policies::nullable {
    struct policy_group_tag;

    template<typename Pointee, typename Metadata>
    struct yes {
        using policy_group = policy_group_tag;

    protected:
        [[nodiscard]] constexpr typename Metadata::pointer validate_by_nullability(typename Metadata::pointer source) { return source; }
    };

    template<typename Pointee, typename Metadata>
    struct no {
        using policy_group = policy_group_tag;

        //================================================================================
        // Deleted Constructors and Assignment Operators: Non-Null Invariant
        //================================================================================

        ///@brief Deleted default constructor to prevent sources of null initialization.
        no() =
            delete /*("Default constructor deleted to prevent null initialization. Use `std::optional<ptr_type<T>>` for default-constructible optional pointers.")*/
            ;

        ///@brief Deleted constructor from `nullptr` to prevent sources of null initialization.
        no(std::nullptr_t) =
            delete /*("Constructor from `nullptr` deleted to prevent null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from `nullptr` to prevent sources of invalid null rebinding.
        no& operator=(std::nullptr_t) =
            delete /*("Assignment from `nullptr` deleted to prevent null rebinding. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        //================================================================================
        // Validation of Non-Null Invariant
        //================================================================================
    protected:
        [[nodiscard]] constexpr typename Metadata::pointer validate_by_nullability(typename Metadata::pointer source)
        {
            if (source == nullptr) [[unlikely]]
                throw std::invalid_argument("`non_nullable` pointers cannot be constructed or assigned from a null pointer.");
            return source;
        }
    }; //struct no
} //namespace base::vocab::inline ptr::ptr_policies
