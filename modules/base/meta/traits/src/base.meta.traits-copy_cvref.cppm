// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.traits-copy_cvref.cppm
 * @version 0.0.2
 * @date June 30, 2026
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
 * @brief Type transformation traits to copy the reference value category and cv-qualifications between types.
 */

//Module partition interface unit
export module base.meta.traits:copy_cvref;

import std;

export namespace base::meta::traits::inline transformation {
    ///@brief Apply the const qualification of `Source` to `Target`.
    template<typename Source, typename Target>
    using copy_const_t = std::conditional_t<std::is_const_v<Source>, std::add_const_t<Target>, Target>;

    ///@brief Apply the volatile qualification of `Source` to `Target`.
    template<typename Source, typename Target>
    using copy_volatile_t = std::conditional_t<std::is_volatile_v<Source>, std::add_volatile_t<Target>, Target>;

    ///@brief Apply the cv-qualifications of `Source` to `Target`.
    template<typename Source, typename Target>
    using copy_cv_t = copy_volatile_t<Source, copy_const_t<Source, Target>>;

    ///@brief Apply the reference value category of `Source` to `Target`.
    template<typename Source, typename Target>
    using copy_reference_t = std::conditional_t<
        std::is_lvalue_reference_v<Source>,
        std::add_lvalue_reference_t<Target>,
        std::conditional_t<std::is_rvalue_reference_v<Source>, std::add_rvalue_reference_t<Target>, Target>
    >;

    ///@brief Apply the reference value category and cv-qualifications of `Source` to `Target`.
    template<typename Source, typename Target>
    using copy_cvref_t = copy_reference_t<Source, copy_cv_t<std::remove_reference_t<Source>, Target>>;
} //namespace base::meta::traits::inline transformation
