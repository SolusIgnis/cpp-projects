// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.sequences-core.cppm
 * @version 0.0.1
 * @date May 18, 2026
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
 * @brief Core type templates and traits.
 * @details
 * :core
​ *  ├── type_list
 * ​ ├── value_list
​ *  ├── uniform_value_list
 * ​ ├── is_type_list_v
 * ​ ├── is_value_list_v
 * ​ ├── is_uniform_value_list_v
 * ​ ├── type_constant
​ *  ├── value_constant
 *  ├── sequence
 *  ├── type_sequence
 *  └── value_sequence
 */

//Module partition interface unit
export module base.meta.sequences:core;

import std;

export namespace base::meta::sequences {
    template<typename... Types>
    struct type_list {};

    template<auto... Values>
    struct value_list {};

    template<typename T, T... Values>
    struct uniform_value_list {
        using as_value_list = value_list<Values...>;
    };
} //namespace base::meta::sequences

namespace base::meta::sequences {
    ///@internal
    template<typename T>
    struct type_constant {
        using type = T;
    };

    ///@internal
    template<auto Value>
    struct value_constant {
        static constexpr auto value = Value;
        using value_type            = decltype(Value);
    };
} //namespace base::meta::sequences

namespace base::meta::sequences {
    template<typename T>
    struct is_type_list : std::false_type {};

    template<typename... Types>
    struct is_type_list<type_list<Types...>> : std::true_type {};

    export template<typename T>
    inline constexpr bool is_type_list_v = is_type_list<std::remove_cvref_t<T>>::value;
} //namespace base::meta::sequences

namespace base::meta::sequences {
    template<typename T>
    struct is_value_list : std::false_type {};

    template<auto... Values>
    struct is_value_list<value_list<Values...>> : std::true_type {};

    template<typename T, T... Values>
    struct is_value_list<uniform_value_list<T, Values...>> : std::true_type {};

    export template<typename T>
    inline constexpr bool is_value_list_v = is_value_list<std::remove_cvref_t<T>>::value;
} //namespace base::meta::sequences

namespace base::meta::sequences {
    template<typename T>
    struct is_uniform_value_list : std::false_type {};

    template<typename T, T... Values>
    struct is_uniform_value_list<uniform_value_list<T, Values...>> : std::true_type {};

    export template<typename T>
    inline constexpr bool is_uniform_value_list_v = is_uniform_value_list<std::remove_cvref_t<T>>::value;
} //namespace base::meta::sequences

export namespace base::meta::sequences {
    template<typename T>
    concept type_sequence = is_type_list_v<T>;

    template<typename T>
    concept value_sequence = is_value_list_v<T>;

    template<typename T>
    concept sequence = type_sequence<T> || value_sequence<T>;
} //namespace base::meta::sequences
