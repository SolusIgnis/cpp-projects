// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-dependency_ptr.cppm
 * @version 0.2.0
 * @date March 11, 2026
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
 * @brief dependency_ptr
 */

//Module partition interface unit
export module base.vocab.ptr:dependency_ptr;

import std;

export namespace base::vocab::inline ptr {
    template<typename T>
        requires (!std::is_reference_v<T>)
    class dependency_ptr {
    public:
        using element_type     = T;
        using value_type       = std::remove_cv_t<T>;
        using pointer          = T*;
        using reference        = T&;
        using rvalue_reference = T&&;
        using difference_type  = std::ptrdiff_t;
    private:
        pointer ptr_;
    public:   
        constexpr explicit dependency_ptr(reference other) noexcept : ptr_(&other) {}
        dependency_ptr& operator=(reference other) noexcept { ptr_ = &other; return *this; }

        dependency_ptr() = delete;
        dependency_ptr(rvalue_reference) = delete;
        dependency_ptr& operator=(rvalue_reference) = delete;
        dependency_ptr& operator=(nullptr_t) = delete;
        
        [[nodiscard]] bool operator==(dependency_ptr) const noexcept = default;
        auto operator<=>(dependency_ptr) const = delete;
        auto operator<=>(const pointer other) const = delete;
        
        [[nodiscard]] constexpr pointer operator->() const noexcept { return ptr_; }
        [[nodiscard]] constexpr reference operator*() const noexcept { return *ptr_; }
        [[nodiscard]] constexpr pointer get() const noexcept { return ptr_; }
        [[nodiscard]] constexpr explicit(false) operator pointer() const noexcept { return this->get(); }

        dependency_ptr& operator++() = delete;
        dependency_ptr& operator--() = delete;
        dependency_ptr operator++(int) = delete;
        dependency_ptr operator--(int) = delete;
        dependency_ptr& operator+=(difference_type) = delete;
        dependency_ptr& operator-=(difference_type) = delete;
        friend dependency_ptr operator+(dependency_ptr, difference_type) = delete;
        friend dependency_ptr operator+(difference_type, dependency_ptr) = delete;
        friend difference_type operator-(dependency_ptr, dependency_ptr) = delete;
        friend dependency_ptr operator-(dependency_ptr, difference_type) = delete;
    };
    
    template<typename T>
    dependency_ptr(T&) -> dependency_ptr<T>;
} //namespace base::vocab::ptr
