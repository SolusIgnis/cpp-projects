// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-core.cppm
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
 * @brief Core pointer vocabulary infrastructure.
 */

//Module partition interface unit
export module base.vocab.ptr:core;

import std;

import base.meta.sequences;
import base.meta.traits;
import base.meta.concepts;

import :metadata;

#ifdef EXPERIMENTAL_CORE_PARTITION
export import :policies;

namespace base::vocab::inline ptr {
    template<typename T>
    concept VocabPtr = requires { typename T::derived_from_ptr_core; };
} //namespace base::vocab::inline ptr

export namespace base::vocab::inline ptr {
    template<template<typename> typename ConcretePtr, typename Pointee, ptr_policies::PtrPolicyList PolicySet>
        requires (!std::is_reference_v<Pointee> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<Pointee>>)
    class ptr_core : public pointer_metadata<Pointee> {
    public:
        struct derived_from_ptr_core;

    private:
        using policy_set = PolicySet;
        using metadata = pointer_metadata<Pointee>;

        metadata::pointer address_; ///<@brief The stored address used by all concrete pointer types.

    public:
        //================================================================================
        // Construction, Assignment, and Swap
        //================================================================================

        //===== Core =====

        ///@brief (Conversion) Implicitly converts from another pointer according to nested `pointer` type conversions.
        template<typename U>
            requires (!std::same_as<U, T>) && std::convertible_to<std::add_pointer_t<U>, pointer>
        constexpr explicit(false) ptr_core(const ptr_core<U>& source) noexcept
            : address_(source.get())
        {}

        ///@brief (Conversion) Assigns from another pointer according to nested `pointer` type conversions.
        template<typename Self, typename U>
            requires (!std::is_const_v<Self>) && (!std::same_as<U, T>) && std::convertible_to<std::add_pointer_t<U>, pointer>
        constexpr Self& operator=(this Self& self, const ptr_core<U>& source) noexcept
        {
            self.address_ = source.get();
            return self;
        }

        ///@brief Swaps addresses.
        friend constexpr void swap(ptr_core& lhs, ptr_core& rhs) noexcept
        {
            using std::swap;
            swap(lhs.address_, rhs.address_);
        }

        //===== Reference Binding (Allowed) =====

        ///@brief Constructs a pointer bound to an existing object.
        constexpr explicit ptr_core(metadata::reference source) noexcept
            requires ptr_policies::allowed_reference_binding_v<policy_set>
            : address_(std::addressof(source))
        {}

        ///@brief Rebinds the pointer to another object.
        template<typename Self>
            requires (!std::is_const_v<Self>)
        constexpr Self& operator=(this Self& self, reference source) noexcept
            requires ptr_policies::allowed_reference_binding_v<policy_set>
        {
            self.address_ = std::addressof(source);
            return self;
        }

        //===== Pointer Binding (Allowed)  =====

        ///@brief Implicitly converts from a raw `pointer`.
        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        constexpr explicit(false) ptr_core(P&& source)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
            : address_(check_for_null(source))
        {}

        ///@brief Assigns from a raw `pointer`.
        template<typename Self, typename P>
            requires (!std::is_const_v<Self>)
                  && std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        constexpr Self& operator=(this Self& self, P&& source)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        {
            self.address_ = check_for_null(source);
            return self;
        }

        ///@brief Implicitly converts from another pointer-like type.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ptr_core>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                     }
        constexpr explicit(false) ptr_core(const Pointer<Element, Args...>& source)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
            : address_(check_for_null(source.get()))
        {}

        ///@brief Assigns from another pointer-like type.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ptr_core>)
                  && (!std::is_const_v<Self>) && requires(Pointer<Element, Args...> ptr) {
                                                     { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                                                 }
        constexpr Self& operator=(this Self& self, const Pointer<Element, Args...>& source)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        {
            self.address_ = check_for_null(source.get());
            return self;
        }

        //================================================================================
        // Deleted Constructors and Assignment Operators: Non-Null Structural Invariant
        //================================================================================

        //===== Pointer Binding (Forbidden) =====

        ///@brief Deleted constructor from `pointer` to structurally guarantee non-null initialization.
        ptr_core(metadata::pointer)
            requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Constructor from `pointer` deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from `pointer` to structurally guarantee non-null rebinding.
        template<typename Self>
        Self& operator=(this Self&&, metadata::pointer)
            requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Assignment from `pointer` deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted constructor from another pointer-like type to structurally guarantee non-null initialization.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        ptr_core(const Pointer<Element, Args...>&)
            requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Constructor from pointer-like types deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from another pointer-like type to structurally guarantee non-null rebinding.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        Self& operator=(this Self&&, const Pointer<Element, Args...>&)
            requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Assignment from pointer-like types deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Array-to-Pointer Decay
        //================================================================================

        //===== Pointer Binding (Allowed) =====

        ///@brief Deleted constructor from C-array to prevent array-to-pointer decay.
        template<typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        ptr_core(AnyCArray&)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        = delete /*("Constructor from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        ///@brief Deleted assignment from C-array to prevent array-to-pointer decay.
        template<typename Self, typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        Self& operator=(this Self&&, AnyCArray&)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        = delete /*("Assignment from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        //===== Reference Binding (Allowed) =====

        ///@brief Deleted constructor from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        ptr_core(metadata::rvalue_reference)
            requires ptr_policies::allowed_reference_binding_v<policy_set>
        = delete /*("Constructor from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self>
        Self& operator=(this Self&&, metadata::rvalue_reference)
            requires ptr_policies::allowed_reference_binding_v<policy_set>
        = delete /*("Assignment from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        //===== Pointer Binding (Allowed) =====

        ///@brief Deleted constructor from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                         }
        ptr_core(std::add_rvalue_reference_t<Pointer<Element, Args...>>)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        = delete /*("Constructor from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                         }
        Self& operator=(this Self&&, std::add_rvalue_reference_t<Pointer<Element, Args...>>)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        = delete /*("Assignment from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        //===== Reference Binding (Forbidden) =====

        ///@brief Deleted constructor from `const reference` to forbid binding to lvalue or rvalue references.
        ptr_core(const metadata::reference)
            requires ptr_policies::forbidden_reference_binding_v<policy_set>
        = delete /*("Constructor from references deleted by policy `reference_binding::forbidden`. Try constructing from the address directly.")*/
            ;

        ///@brief Deleted assignment from `const references` to forbid binding to lvalue or rvalue references.
        template<typename Self>
        Self& operator=(this Self&&, const metadata::reference)
            requires ptr_policies::forbidden_reference_binding_v<policy_set>
        = delete /*("Assignment from references deleted by policy `reference_binding::forbidden`. Try assigning from the address directly.")*/
            ;

        

    }; //class ptr_core
} //namespace base::vocab::inline ptr
#else
export import :core_policies;

namespace base::vocab::inline ptr {
    template<typename T>
    concept VocabPtr = requires { typename T::derived_from_ptr_core; };
} //namespace base::vocab::inline ptr

export namespace base::vocab::inline ptr {
    template<template<typename> typename ConcretePtr, typename Pointee, template<template<typename> typename, typename> typename... Policies>
        requires (!std::is_reference_v<Pointee> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<Pointee>>)
    class ptr_core : public pointer_metadata<Pointee>, public Policies<ConcretePtr, Pointee>... {
    public:
        struct derived_from_ptr_core;
        friend Policies<ConcretePtr, Pointee>...;

    private:
        using metadata = pointer_metadata<Pointee>;

        metadata::pointer address_; ///<@brief The stored address used by all concrete pointer types.

    protected:
        using Policies<ConcretePtr, Pointee>::resolve_address...;
        using Policies<ConcretePtr, Pointee>::is_constructor_explicit...;
        using Policies<ConcretePtr, Pointee>::validate_by_nullability...;

    public:
        //================================================================================
        // Construction, Assignment, and Swap
        //================================================================================

        ///@brief Using constructor deletions from the policies.
        using Policies<ConcretePtr, Pointee>::Policies...;

        ///@brief Using assignment operator deletions from the policies.
        using Policies<ConcretePtr, Pointee>::operator=...;

        ///@brief (Conversion) Implicitly converts from another `ptr_core` according to underlying pointer conversions.
        template<typename OtherPointee>
            requires (!std::same_as<OtherPointee, Pointee>) && std::convertible_to<std::add_pointer_t<OtherPointee>, typename metadata::pointer>
        constexpr explicit(false) ptr_core(const ConcretePtr<OtherPointee>& source) noexcept : Policies<ConcretePtr, Pointee>(true)..., address_(source.get())
        {}

        ///@brief Constructs a pointer when its new address can be resolved by its policies.
        template<typename... Args>
        constexpr explicit(decltype(is_constructor_explicit(std::declval<Args>()...))::value) ptr_core(Args&&... args)
            noexcept(noexcept(resolve_address(std::forward<Args>(args)...)))
            requires requires { resolve_address(std::forward<Args>(args)...); }
            : Policies<ConcretePtr, Pointee>(true)..., address_{resolve_address(std::forward<Args>(args)...)}
        {}

        ///@brief (Conversion) Assigns from another `ptr_core` according to nested `pointer` type conversions.
        template<typename Self, typename OtherPointee>
            requires (!std::is_const_v<Self>) && (!std::same_as<OtherPointee, Pointee>) && std::convertible_to<std::add_pointer_t<OtherPointee>, typename metadata::pointer>
        constexpr Self& operator=(this Self& self, const ConcretePtr<OtherPointee>& source) noexcept
        {
            self.address_ = source.get();
            return self;
        }

        ///@brief Assigns to a pointer when its new address can be resolved by its policies.
        template<typename Self, typename... Args>
            requires (!std::is_const_v<Self>)
        constexpr Self& operator=(this Self& self, Args&&... args)
            noexcept(noexcept(resolve_address(std::forward<Args>(args)...)))
            requires requires { resolve_address(std::forward<Args>(args)...); }
        {
            self.address_ = resolve_address(std::forward<Args>(args)...);
            return self;
        }

        ///@brief Swaps pointer addresses.
        friend constexpr void swap(ConcretePtr<Pointee>& lhs, ConcretePtr<Pointee>& rhs) noexcept
        {
            using std::swap;
            swap(lhs.address_, rhs.address_);
        }

        //================================================================================
        // Pointer Operations
        //================================================================================

        ///@brief Provides member access to the pointee object.
        [[nodiscard]] constexpr metadata::pointer operator->(this auto&& self) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            return self.address_;
        }

        ///@brief Provides a reference to the pointee object.
        [[nodiscard]] constexpr auto& operator*(this auto&& self) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            return *self.address_;
        }

        ///@brief Returns a raw pointer to the stored address.
        [[nodiscard]] constexpr metadata::pointer get(this auto&& self) noexcept { return self.address_; }

        ///@brief Implicitly converts to the nested `pointer` type.
        [[nodiscard]] constexpr explicit(false) operator typename metadata::pointer() const noexcept { return this->get(); }

        ///@brief Rebinding passes through to assignment.
        template<typename Self, typename P>
        Self& rebind(this Self& self, P&& source) noexcept(std::is_nothrow_assignable_v<Self&, P>)
            requires std::is_assignable_v<Self&, P>
        {
            return self = std::forward<P>(source);
        }

        ///@brief Resets the pointer to a new address.
        template<typename Self, typename P>
            requires (!std::same_as<P, std::nullptr_t>)
        constexpr void reset(this Self& self, P&& source) noexcept(std::is_nothrow_assignable_v<Self&, P>)
            requires std::is_assignable_v<Self&, P>
        {
            self = std::forward<P>(source);
        }

        //================================================================================
        // Stream Output
        //================================================================================

        ///@brief Outputs a pointer address to a `std::basic_ostream`.
        template<typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& stream, const ptr_core& ptr)
        {
            // In order to support pointers to arbitrarily cv-qualified objects:
            // 1. `static_cast` to `const volatile void*` to preserve all qualifiers while converting the pointer to `void*`.
            // 2. `const_cast` to `const void*` to satisfy the inserter's interface which lacks `volatile void*` overloads.
            // This is safe because formatting is a read-only numerical operation on the address.
            return stream << const_cast<const void*>(static_cast<const volatile void*>(ptr.get()));
        }
    }; //class ptr_core
} //namespace base::vocab::inline ptr
#endif

/**
 * @brief Partial specialization of `std::hash` for `VocabPtr`s.
 *
 * @tparam T The concrete pointer type.
 *
 * @remark Hashes the underlying stored address rather than pointee object state or values.
 * @remark Consistent with `ptr_core` equality semantics.
 */
export template<base::vocab::ptr::VocabPtr T>
struct std::hash<T> {
    ///@brief Hashes the pointer based on the underlying address.
    [[nodiscard]] constexpr std::size_t operator()(const T& ptr) const noexcept
    {
        return std::hash<typename T::pointer>{}(ptr.get());
    }
};

/**
 * @brief Partial specialization of `std::formatter` for `VocabPtr`s.
 *
 * @tparam T The concrete pointer type.
 * @tparam CharT The character type used by the format string.
 *
 * @remark Formats the stored address according to the rules for its nested `pointer` type.
 */
export template<base::vocab::ptr::VocabPtr T, typename CharT>
struct std::formatter<T, CharT> : std::formatter<const void*, CharT> {
    ///@brief Formats as a raw pointer to the stored address.
    auto format(const T& ptr, auto& ctx) const
    {
        // In order to support pointers to arbitrarily cv-qualified objects:
        // 1. `static_cast` to `const volatile void*` to preserve all qualifiers while converting the pointer to `void*`.
        // 2. `const_cast` to `const void*` to satisfy the formatter's interface which lacks `volatile void*` specializations.
        // This is safe because formatting is a read-only numerical operation on the address.
        return std::formatter<const void*, CharT>::format(
            const_cast<const void*>(static_cast<const volatile void*>(ptr.get())), ctx
        );
    }
};
