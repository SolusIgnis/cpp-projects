// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-core.cppm
 * @version 0.9.0
 * @date July 8, 2026
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
 * @brief Core infrastructure for policy-driven vocabulary pointer types.
 *
 * @details This partition forms the structural foundation of the vocabulary
 * pointer suite, defining shared facilities including the `ptr_core`
 * implementation, supporting concepts and traits, and interoperability utilities.
 * Behavioral capabilities such as traversability, binding, and nullability are
 * selected through compile-time policy composition, allowing multiple pointer
 * abstractions to share their common implementation details while enforcing
 * distinct semantic invariants.
 *
 * `ptr_core` centralizes address storage, pointer operations, and policy-driven
 * interface selection. `VocabPtr` uses a public nested tag type to identify
 * concrete pointer types derived from `ptr_core` specializations.
 * `CompatibleRawPtr` enables `ptr_core` to constrain raw pointer interactions to
 * address types convertible to the type of its stored address. `is_valid_pointee_v`
 * allows `ptr_core` and the concrete pointer types to validate their pointee
 * template parameter domain. `is_smart_ptr_convertible_to_v` enables `ptr_core` to
 * generally interface with other pointer(-like) types. The `std::hash` and
 * `std::formatter` specializations leverage `VocabPtr` to provide hashing and
 * formatting facilities to all of the concrete vocabulary pointers.
 *
 * This partition is primarily intended for implementers of vocabulary pointer
 * types. End users will typically interact with the concrete pointer abstractions
 * defined in other partitions rather than with `ptr_core` directly.
 *
 * @todo Future Development: Use `= delete("reason")` instead of the C-style comments once the C++26 feature becomes available.
 */

//Module partition interface unit
export module base.vocab.ptr:core;

import std;

import base.meta.traits;
import base.meta.concepts;

export import :policies;

namespace base::vocab::inline ptr {
    /**
     * @brief Detects vocabulary pointer types built on `ptr_core`.
     *
     * @tparam T The type being tested.
     *
     * @details Satisfied when `T` exposes the marker type `derived_from_ptr_core`, which is inherited by all
     * concrete vocabulary pointer specializations.
     *
     * @remark Primarily serves to constrain specializations of generic facilities to the vocabulary pointer domain.
     *
     * @internal
     */
    template<typename T>
    concept VocabPtr = requires { typename T::derived_from_ptr_core; };

    template<typename T>
    concept NullableVocabPtr = VocabPtr<T> && T::is_nullable;

    template<typename T>
    concept AlwaysEngagedVocabPtr = VocabPtr<T> && (!T::is_nullable);

    /**
     * @brief Determines whether a type is a pointer compatible with another pointer.
     *
     * @tparam T The type being tested.
     * @tparam OtherPointer The pointer with which compatibility is being tested.
     *
     * @details Satisfied when `T` is a raw pointer that is convertible after decay to `OtherPointer`.
     *
     * @internal
     */
    template<typename T, typename OtherPointer>
    concept CompatibleRawPtr = std::is_pointer_v<std::remove_cvref_t<T>> && std::convertible_to<std::decay_t<T>, OtherPointer>;

    /**
     * @brief Determines whether a pointer type can be resolved to a given raw address type by `std::to_address`.
     *
     * @tparam T The type being tested.
     * @tparam AddressType The type of the raw address to which the resolved address must be convertible.
     *
     * @details Satisfied when a `T` can be the argument to `std::to_address` and the result of that call is convertible to `AddressType`.
     */
    template<typename T, typename AddressType>
    concept ResolvableToAddress = (std::is_pointer_v<T> || requires { typename std::pointer_traits<T>::element_type; })
                               && requires(const T& ptr) {
                                      { std::to_address(ptr) } -> std::convertible_to<AddressType>;
                                  };

    template<typename T, typename U>
    struct pointer_compatible_with_impl : std::false_type {};

    template<typename T, template<typename...> typename ConcretePtr, typename Pointee>
    struct pointer_compatible_with_impl<T, ConcretePtr<Pointee>> : std::bool_constant<!base::meta::traits::is_type_specialization_of_v<T, ConcretePtr>> {};
    

    template<typename T, typename U>
    concept PointerCompatibleWithImpl = VocabPtr<U>
                                     && ResolvableToAddress<T, typename U::address_type>
                                     && !std::is_array_v<T>
                                     && pointer_compatible_with_impl<T, U>::value;
       
    /**
     * @brief Determines whether a type is a pointer compatible with a specified `VocabPtr`.
     *
     * @tparam T The type being tested.
     * @tparam ConcretePtr The concrete pointer template with which compatibility is being tested.
     * @tparam Pointee The pointee type argument for `ConcretePtr`.
     *
     * @details This concept is satisfied when `T` is a pointer other than `ConcretePtr` that is address-compatible with
     * the concrete pointer specialization `ConcretePtr<Pointee>`. The concept constrains function templates within
     * `ptr_core`, so the expectation is that the argument to `ConcretePtr` will be the `ConcretePtr` parameter from a
     * specialization of `ptr_core` and that the argument to `Pointee` will likewise be its `Pointee` parameter.
     *
     * @note This concept removes reference and cv-qualifications from `T`.
     *
     * @internal
     */
    template<typename T, typename U>
    concept PointerCompatibleWith = PointerCompatibleWithImpl<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

    /**
     * @brief Determines whether a type may be used as a vocabulary pointer pointee.
     *
     * @tparam Pointee The candidate pointee type.
     *
     * @details Rejects reference types because pointing to them is ill-formed and rejects
     * any pointer hierarchies that ultimately resolve to function types because vocabulary
     * pointers model data object addresses rather than callable instructions.
     *
     * @remark Enforces the fundamental domain of valid vocabulary pointer specializations.
     *
     * @internal
     */
    template<typename Pointee>
    inline constexpr bool is_valid_pointee_v =
        (!std::is_reference_v<Pointee> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<Pointee>>);

    /**
     * @brief Detects pointer-like types exposing a compatible `get()` member.
     *
     * @tparam Source The candidate pointer-like type.
     * @tparam Target The required pointer target type.
     *
     * @details Evaluates to `true` when an object of type `Source` has a `get()` method whose result is convertible to `Target`.
     *
     * @remark Facilitates interoperability with other pointer(-like) abstractions without depending on specific library implementations.
     *
     * @internal
     */
    template<typename Source, typename Target>
    inline constexpr bool is_smart_ptr_convertible_to_v = requires(Source ptr) {
                                                              { std::as_const(ptr).get() } -> std::convertible_to<Target>;
                                                          };
} //namespace base::vocab::inline ptr

export namespace base::vocab::inline ptr {
    /**
     * @brief Policy-driven foundational layer for the vocabulary pointer suite.
     *
     * @tparam ConcretePtr The derived pointer class template (CRTP-like template-template pattern).
     * @tparam Pointee The type of the object being pointed to.
     * @tparam PolicySet A `PtrPolicyList` type-list configuration selecting one policy from each of the `ptr_policies::traversal`, `ptr_policies::reference_binding`, `ptr_policies::pointer_binding`, and `ptr_policies::nullability` groups.
     *
     * @details `ptr_core` is a policy-configurable foundation for non-owning vocabulary pointer types. It centralizes
     * stored address management, pointer traits, and universal pointer operations while selectively enabling operations
     * that match the policy-configured semantic contract of the concrete pointer type. As the policies are resolved
     * at template instantiation time, this process introduces no runtime overhead.
     *
     * @see `:policies`
     *
     * @remark Array Decay Prevention: Operations taking raw C-arrays are explicitly intercepted and deleted for pointer-binding configurations to block pointer-decay errors when targeting blocks of contiguous memory.
     * @remark Temporary Binding Prevention: Direct binding from temporary variables (including `rvalue_reference`s and pointer-like temporaries) is structurally blocked using deleted sinks, eliminating a primary vector for dangling pointers.
     * @remark Layout Guarantee: `ptr_core` maintains a strict standard-layout representation, allowing the concrete pointers derived from it to preserve the structural properties of a scalar raw pointer.
     *
     * @remark This interface uses C++23 explicit object parameters (e.g., `this auto&& self`) to avoid cv/ref-qualified overload duplication while preserving correct value-category propagation.
     * @note Deleted overloads deliberately use forwarding explicit object parameters (`this Self&&`) so policy-driven diagnostics take precedence over value-category mismatches during overload resolution.
     */
    template<template<typename> typename ConcretePtr, typename Pointee, ptr_policies::PtrPolicyList PolicySet>
        requires is_valid_pointee_v<Pointee>
    class ptr_core {
    public:
        struct derived_from_ptr_core;

    protected:
        /**
         * @typedef core_type
         * @brief Convenience alias of `ptr_core` specialization available to concrete pointers to name their base type.
         */
        using core_type = ptr_core;

    private:
        /**
         * @typedef policy_set
         * @brief The concrete pointer's policy configuration set.
         */
        using policy_set = PolicySet;

        /**
         * @typedef concrete_ptr_instance
         * @brief The pointee-specialized concrete pointer type.
         */
        using concrete_ptr_instance = ConcretePtr<Pointee>;

        ///@brief Incomplete type used in place of `void &`.
        struct void_reference;

        ///@brief Tag type to access private validated-address constructor.
        struct validated_address_tag {};

    public:
        ///@brief Trait allowing users to query the pointer nullability policy.
        static constexpr bool is_nullable = ptr_policies::nullable_nullability_v<policy_set>;

        /**
         * @typedef element_type
         * @brief The stored element type.
         */
        using element_type = Pointee;

        /**
         * @typedef value_type
         * @brief The unqualified element type (`std::remove_cv_t<element_type>`).
         */
        using value_type = std::remove_cv_t<element_type>;

        /**
         * @typedef address_type
         * @brief The raw pointer type of the stored address (`element_type*`).
         */
        using address_type = std::add_pointer_t<element_type>;

        /**
         * @typedef reference
         * @brief The reference type (`element_type&`).
         * @remark When `element_type` is `void`, uses `void_reference&` because `void` as a function parameter is ill-formed.
         */
        using reference = std::conditional_t<
            std::is_void_v<element_type>,
            std::add_lvalue_reference_t<void_reference>,
            std::add_lvalue_reference_t<element_type>
        >;

        /**
         * @typedef rvalue_reference
         * @brief The rvalue reference type (`element_type&&`).
         *
         * @remark When `element_type` is `void`, uses `void_reference&&` because `void` as a function parameter is ill-formed.
         * @note Used only for deletion of invalid overloads to prevent binding to temporaries.
         */
        using rvalue_reference = std::conditional_t<
            std::is_void_v<element_type>,
            std::add_rvalue_reference_t<void_reference>,
            std::add_rvalue_reference_t<element_type>
        >;

        /**
         * @typedef difference_type
         * @brief Pointer difference type (`std::ptrdiff_t`).
         *
         * @note Provided to model pointer interface even when arithmetic is disabled.
         */
        using difference_type = std::ptrdiff_t;

        /**
         * @typedef rebind
         * @brief Yields a concrete pointer specialization of the same concrete pointer template rebound to a new pointee type.
         * @tparam OtherPointee The pointee type of the rebound concrete pointer specialization.
         */
        template<typename OtherPointee>
        using rebind = ConcretePtr<OtherPointee>;

        /**
         * @typedef iterator_concept
         * @brief STL iterator compatibility type for contiguous iterators.
         * @remark Yields `void` for pointers with the rebinding traversal policy.
         */
        using iterator_concept =
            std::conditional_t<ptr_policies::arithmetic_traversal_v<policy_set>, std::contiguous_iterator_tag, void>;

        /**
         * @typedef iterator_category
         * @brief STL iterator compatibility type for random-access iterators.
         * @remark Yields `void` for pointers with the rebinding traversal policy.
         */
        using iterator_category =
            std::conditional_t<ptr_policies::arithmetic_traversal_v<policy_set>, std::random_access_iterator_tag, void>;

    private:
        address_type address_; ///<@brief The stored address used by all concrete pointer types.

        //================================================================================
        // Construction, Assignment, Swap, and `pointer_to` Factory
        //================================================================================

        //===== Universal Core =====

        ///@brief Constructs from a pre-validated address.
        constexpr explicit ptr_core(validated_address_tag, address_type address) noexcept : address_{address} {}

    public:
        ///@brief (Conversion) Implicitly converts from another `ConcretePtr` specialization according to nested `address_type` type conversions.
        template<typename OtherPointee>
            requires (!std::same_as<OtherPointee, element_type>)
                  && std::convertible_to<std::add_pointer_t<OtherPointee>, address_type>
        constexpr explicit(false) ptr_core(const ConcretePtr<OtherPointee>& source) noexcept : ptr_core{validated_address_tag{}, source.get()}
        {}

        ///@brief (Conversion) Assigns from another `ConcretePtr` specialization according to nested `address_type` type conversions.
        template<typename Self, typename OtherPointee>
            requires (!std::is_const_v<Self>) && (!std::same_as<OtherPointee, element_type>)
                  && std::convertible_to<std::add_pointer_t<OtherPointee>, address_type>
        constexpr Self& operator=(this Self& self, const ConcretePtr<OtherPointee>& source) noexcept
        {
            self.address_ = source.get();
            return self;
        }

        ///@brief Swaps addresses.
        friend constexpr void swap(concrete_ptr_instance& lhs, concrete_ptr_instance& rhs) noexcept
        {
            using std::swap;
            swap(lhs.address_, rhs.address_);
        }

        ///@brief Produces a concrete pointer instance bound to a given object.
        static constexpr concrete_ptr_instance pointer_to(reference object) noexcept
            requires (!std::is_void_v<element_type>)
        {
            return concrete_ptr_instance{validated_address_tag{}, std::addressof(object)};
        }
        
        //===== Reference Binding (Allowed) =====

        ///@brief Constructs a pointer bound to an existing object.
        constexpr explicit ptr_core(reference source) noexcept
            requires (!std::is_void_v<element_type>) && ptr_policies::allowed_reference_binding_v<policy_set>
            : ptr_core{validated_address_tag{}, std::addressof(source)}
        {}

        ///@brief Rebinds the pointer to another object.
        template<typename Self>
            requires (!std::is_const_v<Self>)
        constexpr Self& operator=(this Self& self, reference source) noexcept
            requires (!std::is_void_v<element_type>) && ptr_policies::allowed_reference_binding_v<policy_set>
        {
            self.address_ = std::addressof(source);
            return self;
        }

        //===== Pointer Binding (Allowed)  =====
#ifndef OLD_POINTER_BINDING
        ///@brief Implicitly converts from a raw `address_type`. Explicit when `element_type` is void to avoid implicit conversion chaining.
        template<CompatibleRawPtr<address_type> P>
        constexpr explicit(std::is_void_v<element_type>) ptr_core(P&& source) noexcept(
            noexcept(apply_nullability_policy(std::forward<P>(source)))
        )
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
            : ptr_core{validated_address_tag{}, apply_nullability_policy(std::forward<P>(source))}
        {}

        ///@brief Assigns from a raw `address_type`.
        template<typename Self, CompatibleRawPtr<address_type> P>
            requires (!std::is_const_v<Self>)
        constexpr Self&
            operator=(this Self& self, P&& source) noexcept(noexcept(apply_nullability_policy(std::forward<P>(source))))
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        {
            self.address_ = apply_nullability_policy(std::forward<P>(source));
            return self;
        }

        ///@brief Implicitly converts from another pointer-like type. Explicit when `element_type` is void to avoid implicit conversion chaining.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && is_smart_ptr_convertible_to_v<Pointer<Element, Args...>, address_type>
                     constexpr explicit(std::is_void_v<element_type>) ptr_core(
                         const Pointer<Element, Args...>& source
                     ) noexcept(noexcept(apply_nullability_policy(source.get())))
                         requires ptr_policies::allowed_pointer_binding_v<policy_set>
            : ptr_core{validated_address_tag{}, apply_nullability_policy(source.get())}
        {}

        ///@brief Assigns from another pointer-like type.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && (!std::is_const_v<Self>)
                  && is_smart_ptr_convertible_to_v<Pointer<Element, Args...>, address_type>
                     constexpr Self& operator=(this Self& self, const Pointer<Element, Args...>& source) noexcept(
                         noexcept(apply_nullability_policy(source.get()))
                     )
                         requires ptr_policies::allowed_pointer_binding_v<policy_set>
        {
            self.address_ = apply_nullability_policy(source.get());
            return self;
        }
#else
        ///@brief Implicitly converts from another compatible vocabulary pointer type. Explicit when `element_type` is void to avoid implicit conversion chaining.
        template<typename P>
            requires VocabPtr<std::remove_cvref_t<P>>
                  && (!base::meta::traits::is_type_specialization_of_v<std::remove_cvref_t<P>, ConcretePtr>)
                  && std::convertible_to<typename std::remove_cvref_t<P>::address_type, address_type>
        constexpr explicit(std::is_void_v<element_type>) ptr_core(P&& source) noexcept(noexcept(apply_nullability_policy(static_cast<address_type>(source))))
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
            : ptr_core{validated_address_tag{}, apply_nullability_policy(static_cast<address_type>(source))}
        {}

        ///@brief Assigns from another compatible vocabulary pointer type.
        template<typename Self, typename P>
            requires (!std::is_const_v<Self>)
                  && VocabPtr<std::remove_cvref_t<P>>
                  && (!base::meta::traits::is_type_specialization_of_v<std::remove_cvref_t<P>, ConcretePtr>)
                  && std::convertible_to<typename std::remove_cvref_t<P>::address_type, address_type>
        constexpr Self&
            operator=(this Self& self, P&& source) noexcept(noexcept(apply_nullability_policy(std::to_address(source))))
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        {
            self.address_ = apply_nullability_policy(std::to_address(source));
            return self;
        }

        ///@brief Implicitly converts from a compatible pointer type. Explicit when `element_type` is void to avoid implicit conversion chaining.
        template<typename P>
            requires (!VocabPtr<std::remove_cvref_t<P>>)
                  && (!base::meta::traits::is_type_specialization_of_v<std::remove_cvref_t<P>, ConcretePtr>)
                  && (!std::is_array_v<std::remove_cvref_t<P>>)
                  && ResolvableToAddress<std::remove_cvref_t<P>, address_type>
        constexpr explicit(std::is_void_v<element_type>) ptr_core(P&& source) noexcept(noexcept(apply_nullability_policy(std::to_address(source))))
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
            : ptr_core{validated_address_tag{}, apply_nullability_policy(std::to_address(source))}
        {}

        ///@brief Assigns from a compatible pointer type.
        template<typename Self, typename P>
            requires (!std::is_const_v<Self>)
                  && (!VocabPtr<std::remove_cvref_t<P>>)
                  && (!base::meta::traits::is_type_specialization_of_v<std::remove_cvref_t<P>, ConcretePtr>)
                  && (!std::is_array_v<std::remove_cvref_t<P>>)
                  && ResolvableToAddress<std::remove_cvref_t<P>, address_type>
        constexpr Self&
            operator=(this Self& self, P&& source) noexcept(noexcept(apply_nullability_policy(std::to_address(source))))
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        {
            self.address_ = apply_nullability_policy(std::to_address(source));
            return self;
        }
#endif

        //===== Nullability (Nullable) =====

        ///@brief Default constructor value initializes to null.
        ptr_core() noexcept
            requires ptr_policies::nullable_nullability_v<policy_set>
            : ptr_core{validated_address_tag{}, address_type{}}
        {}

        ///@brief Constructor from `nullptr` initializes to null.
        ptr_core(std::nullptr_t null) noexcept
            requires ptr_policies::nullable_nullability_v<policy_set>
            : ptr_core{validated_address_tag{}, null}
        {}

        ///@brief Assignment from `nullptr` rebinds to null.
        template<typename Self>
            requires (!std::is_const_v<Self>)
        Self& operator=(this Self& self, std::nullptr_t null) noexcept
            requires ptr_policies::nullable_nullability_v<policy_set>
        {
            self.address_ = null;
            return self;
        }

        //================================================================================
        // Deleted Constructors and Assignment Operators: Non-Null Structural Invariant
        //================================================================================

        //===== Nullability (Always Engaged) =====

        ///@brief Deleted default constructor to prevent sources of null initialization.
        ptr_core()
            requires ptr_policies::always_engaged_nullability_v<policy_set>
        = delete /*("Default constructor deleted by policy `nullability::always_engaged` to prevent null initialization. Use `std::optional<ptr_type<T>>` for default-constructible optional pointers.")*/
            ;

        ///@brief Deleted constructor from `nullptr` to prevent sources of null initialization.
        ptr_core(std::nullptr_t)
            requires ptr_policies::always_engaged_nullability_v<policy_set>
        = delete /*("Constructor from `nullptr` deleted to prevent null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from `nullptr` to prevent sources of invalid null rebinding.
        template<typename Self>
        Self& operator=(this Self&&, std::nullptr_t)
            requires ptr_policies::always_engaged_nullability_v<policy_set>
        = delete /*("Assignment from `nullptr` deleted by policy `nullability::always_engaged` to prevent null rebinding. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        //===== Pointer Binding (Forbidden) =====

        ///@brief Deleted constructor from raw pointers to structurally guarantee non-null initialization.
        template<CompatibleRawPtr<address_type> P>
        ptr_core(P)
            requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Constructor from raw pointers deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization by the reference-binding constructor. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from raw pointers to structurally guarantee non-null rebinding.
        template<typename Self, CompatibleRawPtr<address_type> P>
        Self& operator=(this Self&&, P)
            requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Assignment from raw pointers deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null assignment by the reference-binding assignment operator. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted constructor from another pointer-like type to structurally guarantee non-null initialization.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && is_smart_ptr_convertible_to_v<Pointer<Element, Args...>, address_type>
                     ptr_core(const Pointer<Element, Args...>&)
                         requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Constructor from pointer-like types deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization by the reference-binding constructor. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from another pointer-like type to structurally guarantee non-null rebinding.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && is_smart_ptr_convertible_to_v<Pointer<Element, Args...>, address_type>
                     Self& operator=(this Self&&, const Pointer<Element, Args...>&)
                         requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Assignment from pointer-like types deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null assignment by the reference-binding assignment operator. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
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
                      && (!std::convertible_to<std::add_lvalue_reference_t<AnyCArray>, reference>)
        = delete /*("Constructor from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        ///@brief Deleted assignment from C-array to prevent array-to-pointer decay.
        template<typename Self, typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        Self& operator=(this Self&&, AnyCArray&)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
                      && (!std::convertible_to<std::add_lvalue_reference_t<AnyCArray>, reference>)
        = delete /*("Assignment from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        //===== Reference Binding (Allowed) =====

        ///@brief Deleted constructor from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        ptr_core(rvalue_reference)
            requires ptr_policies::allowed_reference_binding_v<policy_set>
        = delete /*("Constructor from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self>
        Self& operator=(this Self&&, rvalue_reference)
            requires ptr_policies::allowed_reference_binding_v<policy_set>
        = delete /*("Assignment from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        //===== Pointer Binding (Allowed) =====

        ///@brief Deleted constructor from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && is_smart_ptr_convertible_to_v<Pointer<Element, Args...>, address_type>
                     ptr_core(std::add_rvalue_reference_t<Pointer<Element, Args...>>)
                         requires ptr_policies::allowed_pointer_binding_v<policy_set>
        = delete /*("Constructor from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && is_smart_ptr_convertible_to_v<Pointer<Element, Args...>, address_type>
                     Self& operator=(this Self&&, std::add_rvalue_reference_t<Pointer<Element, Args...>>)
                         requires ptr_policies::allowed_pointer_binding_v<policy_set>
        = delete /*("Assignment from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        //===== Reference Binding (Forbidden) =====

        ///@brief Deleted constructor from `const reference` to forbid binding to lvalue or rvalue references.
        ptr_core(const reference)
            requires ptr_policies::forbidden_reference_binding_v<policy_set>
        = delete /*("Constructor from references deleted by policy `reference_binding::forbidden`. Try constructing from the address directly.")*/
            ;

        ///@brief Deleted assignment from `const references` to forbid binding to lvalue or rvalue references.
        template<typename Self>
        Self& operator=(this Self&&, const reference)
            requires ptr_policies::forbidden_reference_binding_v<policy_set>
        = delete /*("Assignment from references deleted by policy `reference_binding::forbidden`. Try assigning from the address directly.")*/
            ;

        //================================================================================
        // Pointer Operations
        //================================================================================

        //===== Universal Core =====

        ///@brief Provides member access to the pointee object.
        [[nodiscard]] constexpr address_type operator->(this auto&& self) noexcept
            requires base::meta::concepts::CompletePointee<element_type>
                  && std::is_class_v<element_type>
        {
            return self.address_;
        }

        ///@brief Provides a reference to the pointee object.
        [[nodiscard]] constexpr auto& operator*(this auto&& self) noexcept
            requires base::meta::concepts::CompletePointee<element_type>
        {
            return *self.address_;
        }

        ///@brief Returns a raw pointer to the stored address.
        [[nodiscard]] constexpr address_type get(this auto&& self) noexcept { return self.address_; }

        ///@brief Implicitly converts to the nested `address_type` type.
        [[nodiscard]] constexpr explicit(false) operator address_type() const noexcept { return this->get(); }

        ///@brief Resets the pointer to a new address.
        template<typename Self, typename P>
            requires (!std::same_as<P, std::nullptr_t>)
        constexpr void reset(this Self& self, P&& source) noexcept(std::is_nothrow_assignable_v<Self&, P>)
            requires std::is_assignable_v<Self&, P>
        {
            self = std::forward<P>(source);
        }

        //===== Nullability (Nullable) =====

        ///@brief Reset the pointer to `nullptr`.
        constexpr void reset(this auto& self, std::nullptr_t null = nullptr) noexcept
            requires ptr_policies::nullable_nullability_v<policy_set>
        {
            self = null;
        }

        ///@brief Returns a raw pointer to the stored address while disengaging the pointer.
        [[nodiscard]] constexpr address_type release(this auto&& self) noexcept
            requires ptr_policies::nullable_nullability_v<policy_set>
        {
            return std::exchange(self.address_, nullptr);
        }

        ///@brief Compares equality against `nullptr`.
        [[nodiscard]] friend constexpr bool operator==(const concrete_ptr_instance& ptr, std::nullptr_t null) noexcept
            requires ptr_policies::nullable_nullability_v<policy_set>
        {
            return (ptr.get() == null);
        }

        ///@brief Contextually converts to `bool` to test if the pointer is engaged.
        [[nodiscard]] constexpr explicit operator bool(this auto&& self) noexcept
            requires ptr_policies::nullable_nullability_v<policy_set>
        {
            return (self.get() != nullptr);
        }

        //===== Nullability (Always Engaged) =====

        ///@brief Contextually converts to `bool` to "test" if the pointer is engaged. Always returns `true` to confirm invariant.
        [[nodiscard]] constexpr explicit operator bool(this auto&&) noexcept
            requires ptr_policies::always_engaged_nullability_v<policy_set>
        {
            return true;
        }

        //================================================================================
        // Arithmetic Operators: Implemented for Iteration, Deleted Otherwise
        //================================================================================

        //===== Traversal (Arithmetic) =====

        ///@brief Subscript operator provided solely to comply with random-access iterator requirements.
        [[nodiscard]] [[deprecated(
            "Subscript operator conflates pointers with arrays. Use `*(ptr + offset)` for explicit traversal or consider subscripting the container instead."
        )]]
        constexpr auto& operator[](this auto self, difference_type offset) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return *(self + offset);
        }

        ///@brief Prefix increment: increments the stored address.
        template<typename Self>
        constexpr decltype(auto) operator++(this Self&& self) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            ++self.address_;
            return std::forward<Self>(self);
        }

        ///@brief Prefix decrement: decrements the stored address.
        template<typename Self>
        constexpr decltype(auto) operator--(this Self&& self) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            --self.address_;
            return std::forward<Self>(self);
        }

        ///@brief Postfix increment: increments the stored address but returns a pointer to the prior stored address.
        template<typename Self>
        constexpr auto operator++(this Self&& self, int) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            std::decay_t<Self> old{self};
            ++self;
            return old;
        }

        ///@brief Postfix decrement: decrements the stored address but returns a pointer to the prior stored address.
        template<typename Self>
        constexpr auto operator--(this Self&& self, int) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            std::decay_t<Self> old{self};
            --self;
            return old;
        }

        ///@brief Addition assignment: increments the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator+=(this Self&& self, difference_type offset) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            self.address_ += offset;
            return std::forward<Self>(self);
        }

        ///@brief Subtraction assignment: decrements the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator-=(this Self&& self, difference_type offset) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            self.address_ -= offset;
            return std::forward<Self>(self);
        }

        ///@brief Pointer addition: gets a pointer to an address a given distance after the stored address.
        friend constexpr concrete_ptr_instance operator+(concrete_ptr_instance ptr, difference_type offset) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return ptr += offset;
        }

        ///@brief Pointer addition (commutative): gets a pointer to an address a given distance after the stored address.
        friend constexpr concrete_ptr_instance operator+(difference_type offset, concrete_ptr_instance ptr) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return ptr += offset;
        }

        ///@brief Pointer subtraction: gets a pointer to an address a given distance before the stored address.
        friend constexpr concrete_ptr_instance operator-(concrete_ptr_instance ptr, difference_type offset) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return ptr -= offset;
        }

        ///@brief Pointer subtraction (difference): computes the distance between the addresses stored in two pointers.
        friend constexpr difference_type operator-(concrete_ptr_instance lhs, concrete_ptr_instance rhs) noexcept
            requires base::meta::concepts::CompletePointee<element_type> && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return lhs.get() - rhs.get();
        }

        //===== Traversal (Rebinding) =====

        ///@brief Deleted prefix increment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator++(this Self&&)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Prefix increment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted prefix decrement to prevent misuse as an iterator.
        template<typename Self>
        Self& operator--(this Self&&)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Prefix decrement deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted postfix increment to prevent misuse as an iterator.
        template<typename Self>
        Self operator++(this Self&&, int)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Postfix increment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted postfix decrement to prevent misuse as an iterator.
        template<typename Self>
        Self operator--(this Self&&, int)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Postfix decrement deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted addition assignment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator+=(this Self&&, difference_type)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Addition assignment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted subtraction assignment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator-=(this Self&&, difference_type)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Subtraction assignment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend concrete_ptr_instance operator+(concrete_ptr_instance, difference_type)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Pointer addition deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend concrete_ptr_instance operator+(difference_type, concrete_ptr_instance)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Pointer addition deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend difference_type operator-(concrete_ptr_instance, concrete_ptr_instance)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Pointer subtraction deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend concrete_ptr_instance operator-(concrete_ptr_instance, difference_type)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Pointer subtraction deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        //================================================================================
        // Comparison Operators (Three-way for Arithmetic Traversal,  Equality for Rebinding)
        //================================================================================

        //===== Traversal (Arithmetic) =====

        ///@brief Compares in terms of pointer identity.
        [[nodiscard]] friend constexpr auto
            operator<=>(const concrete_ptr_instance& lhs, const concrete_ptr_instance& rhs) noexcept
            requires ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return (lhs.get() <=> rhs.get());
        }

        ///@brief Compares with raw pointer in terms of pointer identity.
        [[nodiscard]] friend constexpr auto operator<=>(const concrete_ptr_instance& ptr, const address_type raw) noexcept
            requires ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return (ptr.get() <=> raw);
        }

        ///@brief Covariantly compares in terms of pointer identity.
        template<std::derived_from<element_type> DerivedT>
            requires (!std::same_as<DerivedT, element_type>)
        [[nodiscard]] friend constexpr auto
            operator<=>(const concrete_ptr_instance& lhs, const ConcretePtr<DerivedT>& rhs) noexcept
            requires ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return (lhs.get() <=> rhs.get());
        }

        ///@brief Covariantly compares a `ConcretePtr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<element_type> DerivedT>
        [[nodiscard]] friend constexpr auto
            operator<=>(const concrete_ptr_instance& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
            requires ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return (lhs.get() <=> rhs);
        }

        ///@brief Covariantly compares a raw pointer-to-base with a `ConcretePtr`-to-derived in terms of pointer identity.
        template<std::derived_from<element_type> DerivedT>
        [[nodiscard]] friend constexpr auto operator<=>(const address_type lhs, const ConcretePtr<DerivedT>& rhs) noexcept
            requires ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return (lhs <=> rhs.get());
        }

        //===== Nullability (Always Engaged) =====

        ///@brief Deleted comparison against `nullptr`.
        [[nodiscard]] friend constexpr bool operator==(const concrete_ptr_instance&, std::nullptr_t) noexcept
            requires ptr_policies::always_engaged_nullability_v<policy_set>
        = delete;

        //===== Traversal (Rebinding) =====

        ///@brief Compares equality in terms of pointer identity.
        [[nodiscard]] friend constexpr bool
            operator==(const concrete_ptr_instance& lhs, const concrete_ptr_instance& rhs) noexcept
            requires ptr_policies::rebinding_traversal_v<policy_set>
        {
            return (lhs.get() == rhs.get());
        }

        ///@brief Covariantly compares equality in terms of pointer identity.
        template<std::derived_from<element_type> DerivedT>
            requires (!std::same_as<DerivedT, element_type>)
        [[nodiscard]] friend constexpr bool
            operator==(const concrete_ptr_instance& lhs, const ConcretePtr<DerivedT>& rhs) noexcept
            requires ptr_policies::rebinding_traversal_v<policy_set>
        {
            return (lhs.get() == rhs.get());
        }

        ///@brief Covariantly compares equality of an `ConcretePtr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<element_type> DerivedT>
        [[nodiscard]] friend constexpr bool
            operator==(const concrete_ptr_instance& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
            requires ptr_policies::rebinding_traversal_v<policy_set>
        {
            return (lhs.get() == rhs);
        }

        ///@brief Covariantly compares equality of a raw pointer-to-base with an `ConcretePtr`-to-derived in terms of pointer identity.
        template<std::derived_from<element_type> DerivedT>
        [[nodiscard]] friend constexpr bool operator==(const address_type lhs, const ConcretePtr<DerivedT>& rhs) noexcept
            requires ptr_policies::rebinding_traversal_v<policy_set>
        {
            return (lhs == rhs.get());
        }

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        template<typename Self>
        auto operator<=>(this Self&&, Self)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Comparison operators deleted by policy `traversal::rebinding` to prevent address comparisons. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        auto operator<=>(const address_type) const
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Comparison operators deleted by policy `traversal::rebinding` to prevent address comparisons. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        //================================================================================
        // Stream Output
        //================================================================================

        ///@brief Outputs a `ptr_core` address to a `std::basic_ostream`.
        template<typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits>&
            operator<<(std::basic_ostream<CharT, Traits>& stream, const concrete_ptr_instance& ptr)
        {
            // In order to support pointers to arbitrarily cv-qualified objects:
            // 1. `static_cast` to `const volatile void*` to preserve all qualifiers while converting the pointer to `void*`.
            // 2. `const_cast` to `const void*` to satisfy the inserter's interface which lacks `volatile void*` overloads.
            // This is safe because formatting is a read-only numerical operation on the address.
            return stream << const_cast<const void*>(static_cast<const volatile void*>(ptr.get()));
        }

    private:
        ///@brief Enforces the non-null invariant for `nullability::always_engaged` pointers by only passing the address through when it is not null but allows unchecked pass-through otherwise.
        [[nodiscard]] static constexpr address_type
            apply_nullability_policy(address_type source) noexcept(!ptr_policies::always_engaged_nullability_v<policy_set>)
        {
            if constexpr (ptr_policies::always_engaged_nullability_v<policy_set>) {
                if (source == nullptr) [[unlikely]] {
                    throw std::invalid_argument(
                        "`nullability::always_engaged` pointers cannot be constructed or assigned from a null pointer."
                    );
                }
            }
            return source;
        }
    }; //class ptr_core

    /**
     * @fn constexpr explicit ptr_core(validated_address_tag, address_type address) noexcept
     *
     * @param address The validated raw address to store.
     *
     * @post `get() == address`.
     *
     * @details This constructor directly stores the provided address without any
     * validation. User-facing constructors or factory functions perform runtime or
     * compile-time validation and forward to this constructor to initialize the
     * stored address.
     *
     * @note Enabled regardless of policy selections as an underlying common implication detail.
     * @internal
     */
    /**
     * @overload constexpr explicit(false) ptr_core(const ConcretePtr<OtherPointee>& source) noexcept
     *
     * @tparam OtherPointee The source pointee type whose address is convertible to `address_type`.
     *
     * @param source The pointer being converted.
     *
     * @pre If the selected policies include `nullability::always_engaged`, `source` must not be null.
     * @pre If not null, `source` must point to a valid object that outlives the resulting pointer.
     * @post `get() == source.get()`.
     *
     * @details This single constructor handles:
     * 1. Derived-to-Base conversion (e.g., `ptr<Derived>` to `ptr<Base>`).
     * 2. Qualification conversion (e.g., `ptr<T>` to `ptr<const T>`).
     * 3. Type erasure (e.g., `ptr<T>` to `ptr<void>`).
     *
     * @remark Preserves covariance. Converts from pointer-to-derived to pointer-to-base implicitly.
     * @remark Enables type erasure by converting pointer-to-`OtherPointee` to pointer-to-`void` when applicable.
     * @remark Enables conversion between compatible specializations of the same concrete pointer type.
     * @note Enabled regardless of policy selections because conversion between different pointee specializations of the same concrete pointer type preserves all class invariants.
     */
    /**
     * @overload constexpr explicit ptr_core(reference source) noexcept
     *
     * @param source The object to reference.
     *
     * @pre `source` must refer to a valid object that outlives the constructed pointer.
     * @post `get() == std::addressof(source)`.
     *
     * @details Binds the stored address directly to an existing `source` object
     * without affecting ownership or pointee lifetime.
     *
     * @remark Prevents binding to temporaries via a deleted rvalue-reference overload.
     * @note Enabled by policy `reference_binding::allowed`.
     */
    /**
     * @overload constexpr explicit(std::is_void_v<element_type>) ptr_core(P&& source)
     *
     * @tparam P A raw pointer type which must decay to a type convertible to `address_type`.
     *
     * @param source The raw pointer to bind.
     *
     * @pre If the selected policies include `nullability::always_engaged`, `source` must not be null.
     * @pre If not null, `source` must point to a valid object that outlives the resulting pointer.
     * @post `get() == static_cast<address_type>(source)`.
     *
     * @throws std::invalid_argument if the selected policies include `nullability::always_engaged` and `source == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @details Stores the supplied address after applying the configured nullability
     * policy. Captures the original argument type prior to decay through a forwarding
     * reference so that C-array arguments can be diagnosed explicitly rather than
     * silently decaying to element pointers.
     *
     * @note This constructor is constrained to raw pointers to prevent hijacking copy/move operations or accepting arrays.
     * @remark Explicit when `element_type` is `void` to prevent unintended implicit type erasure conversion chains.
     * @remark Binds the stored address without affecting ownership or pointee lifetime.
     * @note Enabled by policy `pointer_binding::allowed`.
     */
    /**
     * @overload constexpr explicit(std::is_void_v<element_type>) ptr_core(const Pointer<Element, Args...>& source)
     *
     * @tparam Pointer A pointer-like class template exposing `get()`.
     * @tparam Element The element type of the source pointer-like type.
     * @tparam Args Additional template parameters of the source pointer-like type.
     *
     * @param source The pointer-like object providing access to a pointer-compatible address value.
     *
     * @pre `source.get()` must be a valid expression convertible to `address_type`.
     * @pre If the selected policies include `nullability::always_engaged`, `source.get()` must not be null.
     * @pre If not null, `source.get()` must point to a valid object that outlives the resulting pointer.
     * @post `get() == static_cast<address_type>(source.get())`.
     *
     * @throws std::invalid_argument if the selected policies include `nullability::always_engaged` and `source.get() == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @details Stores the supplied address after applying the configured nullability
     * policy. Supports covariance, qualification conversion, and type erasure.
     *
     * @remark Enables interoperation with pointer-like types that expose a `get()` member.
     * @remark The source type must not be a specialization of `ConcretePtr` (to avoid ambiguity with existing overloads).
     * @remark Explicit when `element_type` is `void` to prevent unintended implicit type erasure conversion chains.
     * @remark This constructor does not transfer ownership nor affect the lifetime of the pointee object.
     * @note Enabled by policy `pointer_binding::allowed`.
     */
    /**
     * @overload constexpr ptr_core() noexcept
     *
     * @post `get() == nullptr`.
     *
     * @remark Default-constructs a disengaged pointer by value initialization of the stored address.
     * @note Enabled by policy `nullability::nullable`.
     */
    /**
     * @overload constexpr ptr_core(std::nullptr_t null) noexcept
     *
     * @param null A `nullptr` literal.
     *
     * @post `get() == nullptr`.
     *
     * @remark Constructs a disengaged pointer.
     * @note Enabled by policy `nullability::nullable`.
     */
    /**
     * @fn constexpr Self& operator=(this Self& self, const ConcretePtr<OtherPointee>& source) noexcept
     *
     * @tparam Self The non-const concrete pointer type deduced from the call site.
     * @tparam OtherPointee The source pointee type whose address is convertible to `address_type`.
     *
     * @param self The pointer being rebound.
     * @param source The pointer being converted.
     *
     * @return Reference to `self`.
     *
     * @pre If the selected policies include `nullability::always_engaged`, `source` must not be null.
     * @pre If not null, `source` must point to a valid object that outlives the resulting pointer.
     * @post `self.get() == source.get()`.
     *
     * @details This single assignment operator handles:
     * 1. Derived-to-Base conversion (e.g., `ptr<Derived>` to `ptr<Base>`).
     * 2. Qualification conversion (e.g., `ptr<T>` to `ptr<const T>`).
     * 3. Type erasure (e.g., `ptr<T>` to `ptr<void>`).
     *
     * @remark Preserves covariance. Converts from pointer-to-derived to pointer-to-base implicitly.
     * @remark Enables type erasure by converting pointer-to-`OtherPointee` to pointer-to-`void` when applicable.
     * @remark Enables conversion between compatible specializations of the same concrete pointer type.
     * @note Enabled regardless of policy selections because conversion between different pointee specializations of the same concrete pointer type preserves all class invariants.
     */
    /**
     * @overload constexpr Self& operator=(this Self& self, reference source) noexcept
     *
     * @tparam Self The non-const concrete pointer type deduced from the call site.
     *
     * @param self The pointer being rebound.
     * @param source The object to reference.
     *
     * @return Reference to `self`.
     *
     * @pre `source` must refer to a valid object that outlives the pointer.
     * @post `self.get() == std::addressof(source)`.
     *
     * @remark Rebinds the stored address directly to an existing object without affecting ownership or pointee lifetime.
     * @remark Prevents binding to temporaries via a deleted rvalue-reference overload.
     * @note Enabled by policy `reference_binding::allowed`.
     */
    /**
     * @overload constexpr Self& operator=(this Self& self, P&& source)
     *
     * @tparam Self The non-const concrete pointer type deduced from the call site.
     * @tparam P A raw pointer type which must decay to a type convertible to `address_type`.
     *
     * @param self The pointer being rebound.
     * @param source The raw pointer to bind.
     *
     * @return Reference to `self`.
     *
     * @pre If the selected policies include `nullability::always_engaged`, `source` must not be null.
     * @pre If not null, `source` must point to a valid object that outlives the resulting pointer.
     * @post `self.get() == static_cast<address_type>(source)`.
     *
     * @throws std::invalid_argument if the selected policies include `nullability::always_engaged` and `source == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @details Stores the supplied address after applying the configured nullability
     * policy. Captures the original argument type prior to decay through a forwarding
     * reference so that C-array arguments can be diagnosed explicitly rather than
     * silently decaying to element pointers.
     *
     * @note This assignment operator is constrained to raw pointers to prevent hijacking copy/move operations or accepting arrays.
     * @remark Rebinds the stored address without affecting ownership or pointee lifetime.
     * @note Enabled by policy `pointer_binding::allowed`.
     */
    /**
     * @overload constexpr Self& operator=(this Self& self, const Pointer<Element, Args...>& source)
     *
     * @tparam Pointer A pointer-like class template exposing `get()`.
     * @tparam Element The element type of the source pointer-like type.
     * @tparam Args Additional template parameters of the source pointer-like type.
     *
     * @param source The pointer-like object providing access to a pointer-compatible address value.
     *
     * @return Reference to `self`.
     *
     * @pre `source.get()` must be a valid expression convertible to `address_type`.
     * @pre If the selected policies include `nullability::always_engaged`, `source.get()` must not be null.
     * @pre If not null, `source.get()` must point to a valid object that outlives the resulting pointer.
     * @post `self.get() == static_cast<address_type>(source.get())`.
     *
     * @throws std::invalid_argument if the selected policies include `nullability::always_engaged` and `source.get() == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @remark Rebinds the stored address without affecting ownership or pointee lifetime.
     * @note Enabled by policy `pointer_binding::allowed`.
     */
    /**
     * @overload constexpr Self& operator=(this Self& self, std::nullptr_t null) noexcept
     *
     * @tparam Self The non-const concrete pointer type deduced from the call site.
     *
     * @param self The pointer being rebound.
     * @param null A `nullptr` literal.
     *
     * @return Reference to `self`.
     *
     * @post `self.get() == nullptr`.
     *
     * @remark Disengages the pointer.
     * @note Enabled by policy `nullability::nullable`.
     */
    /**
     * @fn constexpr void swap(concrete_ptr_instance& lhs, concrete_ptr_instance& rhs) noexcept
     *
     * @param lhs The first pointer object.
     * @param rhs The second pointer object.
     *
     * @post The stored addresses of `lhs` and `rhs` are exchanged.
     *
     * @remark Swaps only the stored addresses.
     * @remark Does not affect ownership or pointee lifetime.
     * @remark Provided as a hidden friend for ADL interoperability.
     */
    /**
     * @fn constexpr pointer operator->(this auto&& self) noexcept
     *
     * @param self The pointer providing member access.
     *
     * @return A raw `address_type` to the stored address for use by the built-in member access operator.
     *
     * @pre If the selected policies include `nullability::nullable`, `self.get() != nullptr`.
     * @pre The stored address points to a valid object.
     *
     * @remark Provides pointee member access semantics.
     * @note Available only when `element_type` satisfies `base::meta::concepts::CompletePointee` (i.e. is a complete object type).
     */
    /**
     * @fn constexpr auto& operator*(this auto&& self) noexcept
     *
     * @param self The pointer being dereferenced.
     *
     * @return Reference to the pointee object.
     *
     * @pre If the selected policies include `nullability::nullable`, `self.get() != nullptr`.
     * @pre The stored address points to a valid object.
     *
     * @note Available only when `element_type` satisfies `base::meta::concepts::CompletePointee` (i.e. is a complete object type).
     */
    /**
     * @fn constexpr pointer get(this auto&& self) noexcept
     *
     * @param self The pointer exposing its stored address.
     *
     * @post If the selected policies include `nullability::always_engaged`, the returned pointer is non-null.
     *
     * @return A raw `address_type` to the stored address.
     *
     * @remark Provided for interoperability with pointer-based APIs.
     */
    /**
     * @fn constexpr operator pointer() const noexcept
     *
     * @post If the selected policies include `nullability::always_engaged`, the returned pointer is non-null.
     *
     * @return A raw `address_type` to the stored address.
     *
     * @remark Enables seamless interoperability with legacy interfaces expecting raw pointers.
     * @warning Implicit conversion may obscure pointer semantics; prefer `get()` when clarity is important.
     */
    /**
     * @fn constexpr void reset(this Self& self, P&& source)
     *
     * @tparam Self The non-const concrete pointer type deduced from the call site.
     * @tparam P A source type assignable to the concrete pointer.
     *
     * @param self The pointer being rebound.
     * @param source The source used to replace the stored address.
     *
     * @post Equivalent to `self = std::forward<P>(source)`.
     *
     * @remark Replaces the stored address without affecting ownership or pointee lifetime.
     * @remark Convenience wrapper over assignment for generic pointer-like interoperability.
     */
    /**
     * @overload constexpr void reset(this auto& self, std::nullptr_t null = nullptr) noexcept
     *
     * @param self The pointer being disengaged.
     * @param null A `nullptr` literal.
     *
     * @post `self.get() == nullptr`.
     *
     * @note Due to the default argument to the `null` parameter, this provides nullary invocable ergonomics for the simple null-reset operation.
     * @remark Disengages the pointer by resetting the stored address to `nullptr`.
     * @note Enabled by policy `nullability::nullable`.
     */
    /**
     * @fn constexpr pointer release(this auto&& self) noexcept
     *
     * @param self The pointer exposing its stored address and being disengaged.
     *
     * @return The previously stored address.
     *
     * @post `self.get() == nullptr`.
     *
     * @remark Returns the stored address and disengages the pointer.
     * @remark Does not affect ownership or pointee lifetime.
     * @remark Provided for interoperability with generic pointer-like interfaces.
     * @note Enabled by policy `nullability::nullable`.
     */
    /**
     * @fn constexpr explicit operator bool(this auto&& self) noexcept
     *
     * @return `true` if the pointer is engaged; otherwise `false`.
     *
     * @remark Satisfies boolean-testable requirements in generic templates and logical contexts.
     * @remark Models traditional pointer engagement semantics through contextual conversion to `bool`.
     * @note Enabled by policy `nullability::nullable`.
     */
    /**
     * @overload constexpr explicit operator bool(this auto&& self) noexcept
     *
     * @return `true`. (The pointer is guaranteed by invariant to always be engaged.)
     *
     * @post Always evaluates to `true`.
     *
     * @remark Satisfies boolean-testable requirements in generic templates and logical contexts.
     * @remark Models pointer interface through contextual conversion to `bool` despite the absence of a disengaged state.
     * @note Enabled by policy `nullability::always_engaged`.
     * @note Because this overload always returns `true`, compilers may optimize away engagement checks when the concrete type is known.
     * @note This does not indicate engagement/optionality as policy `nullability::always_engaged` implies no disengaged state.
     */
    /**
     * @fn constexpr bool operator==(const concrete_ptr_instance& lhs, const concrete_ptr_instance& rhs) noexcept
     *
     * @param lhs The left-hand side pointer.
     * @param rhs The right-hand side pointer.
     *
     * @return `true` if both pointers store the same address; otherwise `false`.
     *
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     * @note Enabled by policy `traversal::rebinding`.
     */
    /**
     * @overload constexpr bool operator==(const concrete_ptr_instance& lhs, const ConcretePtr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `element_type`, of the right-hand side pointer.
     *
     * @param lhs The left-hand side pointer.
     * @param rhs The right-hand side pointer.
     *
     * @return `true` if both pointers store the same address; otherwise `false`.
     *
     * @remark Enables equality comparison between pointer specializations of related types when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     * @note Enabled by policy `traversal::rebinding`.
     */
    /**
     * @overload constexpr bool operator==(const concrete_ptr_instance& lhs, std::add_pointer_t<DerivedT> rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `element_type`, of the raw pointer.
     *
     * @param lhs The pointer being compared.
     * @param rhs The raw pointer being compared.
     *
     * @return `true` if `lhs.get() == rhs`; otherwise `false`.
     *
     * @remark Enables equality comparison with raw pointers-to-derived when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     * @note Enabled by policy `traversal::rebinding`.
     */
    /**
     * @overload constexpr bool operator==(const address_type lhs, const ConcretePtr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `element_type`, of the concrete pointer.
     *
     * @param lhs The raw pointer being compared.
     * @param rhs The pointer being compared.
     *
     * @return `true` if `lhs == rhs.get()`; otherwise `false`.
     *
     * @remark Enables comparison with raw pointers-to-base when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     * @note Enabled by policy `traversal::rebinding`.
     */
    /**
     * @overload constexpr bool operator==(const concrete_ptr_instance& ptr, std::nullptr_t null) noexcept
     *
     * @param ptr The pointer being compared.
     * @param null The null pointer value.
     *
     * @return `true` if `ptr` is disengaged; otherwise `false`.
     *
     * @remark Tests engagement state by comparing the stored address against `nullptr`.
     * @remark Equivalent to contextual boolean conversion.
     * @note Enabled by policy `nullability::nullable`.
     */
    /**
     * @fn constexpr auto operator<=>(const concrete_ptr_instance& lhs, const concrete_ptr_instance& rhs) noexcept
     *
     * @param lhs The left-hand side pointer.
     * @param rhs The right-hand side pointer.
     *
     * @return The three-way comparison result of the stored addresses.
     *
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     * @remark Preserves C++ standard raw pointer address ordering semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     */
    /**
     * @overload constexpr auto operator<=>(const concrete_ptr_instance& ptr, const address_type rhs) noexcept
     *
     * @param ptr The vocabulary pointer pointer.
     * @param raw The raw pointer.
     *
     * @return The three-way comparison result of the stored addresses.
     *
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     * @remark Preserves C++ standard raw pointer address ordering semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     */
    /**
     * @overload constexpr auto operator<=>(const concrete_ptr_instance& lhs, const ConcretePtr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `element_type`, of the right-hand side pointer.
     *
     * @param lhs The left-hand side pointer.
     * @param rhs The right-hand side pointer.
     *
     * @return The three-way comparison result of the stored addresses.
     *
     * @remark Enables ordering comparison between pointer specializations of related types when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     * @remark Preserves C++ standard raw pointer address ordering semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     */
    /**
     * @overload constexpr auto operator<=>(const concrete_ptr_instance& lhs, std::add_pointer_t<DerivedT> rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `element_type`, of the raw pointer.
     *
     * @param lhs The pointer being compared.
     * @param rhs The raw pointer being compared.
     *
     * @return The three-way comparison result of the stored addresses.
     *
     * @remark Enables comparison with raw pointers-to-derived when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     * @remark Preserves C++ standard raw pointer address ordering semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     */
    /**
     * @overload constexpr auto operator<=>(const address_type lhs, const ConcretePtr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `element_type`, of the concrete pointer.
     *
     * @param lhs The raw pointer being compared.
     * @param rhs The pointer being compared.
     *
     * @return The three-way comparison result of the stored addresses.
     *
     * @remark Enables comparison with raw pointers-to-base when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     * @remark Preserves C++ standard raw pointer address ordering semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     */
    /**
     * @fn constexpr auto& operator[](this auto self, difference_type offset) noexcept
     *
     * @param self The pointer whose stored address is the base of the offset address.
     * @param offset The offset, in elements, to add to the base address to compute the address to dereference.
     *
     * @return Reference to the pointee object located at `self.get() + offset`.
     *
     * @note Enabled by policy `traversal::arithmetic`.
     * @note This operator is provided solely to fulfill the requirements of `std::random_access_iterator`.
     * @deprecated Applying the subscript operator to a pointer conflates pointers with arrays. Use `*(ptr + offset)` for explicit traversal or consider subscripting the container instead.
     * @warning Accessing beyond the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr decltype(auto) operator++(this Self&& self) noexcept
     *
     * @tparam Self The deduced concrete pointer type.
     *
     * @param self The pointer whose stored address is incremented.
     *
     * @return An lvalue or rvalue reference to `self`, preserving its original value category.
     *
     * @pre The stored address does not point past the end of the referenced contiguous sequence.
     * @post `self.get()` refers to the next contiguous element.
     *
     * @remark Advances the stored address using built-in pointer increment semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     * @warning Advancing beyond the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @overload constexpr auto operator++(this Self&& self, int) noexcept
     *
     * @tparam Self The deduced concrete pointer type.
     *
     * @param self The pointer whose stored address is incremented.
     *
     * @return A copy of the pointer prior to increment.
     *
     * @pre The stored address does not point past the end of the referenced contiguous sequence.
     * @post `self.get()` refers to the next contiguous element.
     *
     * @remark Advances the stored address using built-in pointer increment semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     * @warning Advancing beyond the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr decltype(auto) operator--(this Self&& self) noexcept
     *
     * @tparam Self The deduced concrete pointer type.
     *
     * @param self The pointer whose stored address is decremented.
     *
     * @return An lvalue or rvalue reference to `self`, preserving its original value category.
     *
     * @pre The stored address does not point to the beginning of the referenced contiguous sequence.
     * @post `self.get()` refers to the previous contiguous element.
     *
     * @remark Retreats the stored address using built-in pointer decrement semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     * @warning Decrementing before the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @overload constexpr auto operator--(this Self&& self, int) noexcept
     *
     * @tparam Self The deduced concrete pointer type.
     *
     * @param self The pointer whose stored address is decremented.
     *
     * @return A copy of the pointer prior to decrement.
     *
     * @pre The stored address does not point to the beginning of the referenced contiguous sequence.
     * @post `self.get()` refers to the previous contiguous element.
     *
     * @remark Retreats the stored address using built-in pointer decrement semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     * @warning Decrementing before the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr decltype(auto) operator+=(this Self&& self, difference_type offset) noexcept
     *
     * @tparam Self The deduced concrete pointer type.
     *
     * @param self The pointer whose stored address is advanced.
     * @param offset The signed offset, in elements, to add.
     *
     * @return An lvalue or rvalue reference to `self`, preserving its original value category.
     *
     * @pre If `offset` is positive, the stored address points at least `offset - 1` elements before the end of the referenced contiguous sequence.
     * @pre If `offset` is negative, the stored address points at least `-offset` elements after the beginning of the referenced contiguous sequence.
     * @post `self.get()` refers to the address `offset` elements after its previous value.
     *
     * @remark Advances the stored address using built-in pointer arithmetic semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     * @warning Traversing outside the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr decltype(auto) operator-=(this Self&& self, difference_type offset) noexcept
     *
     * @tparam Self The deduced concrete pointer type.
     *
     * @param self The pointer whose stored address is retreated.
     * @param offset The signed offset, in elements, to subtract.
     *
     * @return An lvalue or rvalue reference to `self`, preserving its original value category.
     *
     * @pre If `offset` is negative, the stored address points at least `(-offset) - 1` elements before the end of the referenced contiguous sequence.
     * @pre If `offset` is positive, the stored address points at least `offset` elements after the beginning of the referenced contiguous sequence.
     * @post `self.get()` refers to the address `offset` elements before its previous value.
     *
     * @remark Retreats the stored address using built-in pointer arithmetic semantics.
     * @note Enabled by policy `traversal::arithmetic`.
     * @warning Traversing outside the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr concrete_ptr_instance operator+(concrete_ptr_instance ptr, difference_type offset) noexcept
     *
     * @param ptr The base pointer.
     * @param offset The signed offset, in elements, to add.
     *
     * @return A new pointer referring to the address `offset` elements after `ptr`.
     *
     * @pre If `offset` is positive, the stored address points at least `offset - 1` elements before the end of the referenced contiguous sequence.
     * @pre If `offset` is negative, the stored address points at least `-offset` elements after the beginning of the referenced contiguous sequence.
     *
     * @remark Computes an offset pointer using built-in pointer arithmetic semantics.
     * @remark Does not modify the original pointer.
     * @note Enabled by policy `traversal::arithmetic`.
     * @warning Traversing outside the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @overload constexpr concrete_ptr_instance operator+(difference_type offset, concrete_ptr_instance ptr) noexcept
     *
     * @param offset The signed offset, in elements, to add.
     * @param ptr The base pointer.
     *
     * @return A new pointer referring to the address `offset` elements after `ptr`.
     *
     * @pre If `offset` is positive, the stored address points at least `offset - 1` elements before the end of the referenced contiguous sequence.
     * @pre If `offset` is negative, the stored address points at least `-offset` elements after the beginning of the referenced contiguous sequence.
     *
     * @remark Provides commutative addition syntax.
     * @remark Does not modify the original pointer.
     * @note Enabled by policy `traversal::arithmetic`.
     * @warning Traversing outside the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr concrete_ptr_instance operator-(concrete_ptr_instance ptr, difference_type offset) noexcept
     *
     * @param ptr The base pointer.
     * @param offset The signed offset, in elements, to subtract.
     *
     * @return A new pointer referring to the address `offset` elements before `ptr`.
     *
     * @pre If `offset` is negative, the stored address points at least `(-offset) - 1` elements before the end of the referenced contiguous sequence.
     * @pre If `offset` is positive, the stored address points at least `offset` elements after the beginning of the referenced contiguous sequence.
     *
     * @remark Computes an offset pointer using built-in pointer arithmetic semantics.
     * @remark Does not modify the original pointer.
     * @note Enabled by policy `traversal::arithmetic`.
     * @warning Traversing outside the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr difference_type operator-(concrete_ptr_instance lhs, concrete_ptr_instance rhs) noexcept
     *
     * @param lhs The left-hand side pointer.
     * @param rhs The right-hand side pointer.
     *
     * @return The distance, in elements, between the stored addresses.
     *
     * @pre `lhs` and `rhs` point into the same contiguous sequence.
     *
     * @remark Equivalent to `lhs.get() - rhs.get()`.
     * @remark The result is positive when `lhs` refers to a later element than `rhs`.
     * @note Enabled by policy `traversal::arithmetic`.
     * @warning Subtracting pointers that do not point into the same contiguous sequence results in undefined behavior.
     */
    /**
     * @fn std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& stream, const concrete_ptr_instance& ptr)
     *
     * @tparam CharT The character type of the stream.
     * @tparam Traits The character traits type of the stream.
     *
     * @param stream The destination output stream.
     * @param ptr The pointer whose stored address is to be formatted.
     *
     * @return `stream`.
     *
     * @remark Outputs the stored address using the standard pointer formatting rules of the stream.
     * @remark Formats pointer identity only; the pointee object is not inspected.
     * @remark Preserves cv-qualification during conversion to `const void*` before insertion.
     * @remark Provided as a hidden friend to enable argument-dependent lookup.
     */
    /**
     * @fn static constexpr address_type apply_nullability_policy(address_type source) noexcept(!ptr_policies::always_engaged_nullability_v<policy_set>)
     *
     * @param source The address to validate.
     *
     * @return The same address if valid according to the selected `nullability` policy.
     *
     * @pre The selected policies include `nullability::nullable`, or `source != nullptr`.
     * @post If the selected policies include `nullability::always_engaged`, the returned pointer is guaranteed to be non-null.
     *
     * @throws std::invalid_argument if the selected policies include `nullability::always_engaged` and `source == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @remark Centralizes enforcement of the non-null invariant when the selected policies include `nullability::always_engaged` for all constructors and assignment operators accepting raw or pointer-like inputs.
     * @note This function does not perform lifetime validation; it assumes the caller ensures the pointee remains valid.
     * @note Enabled regardless of policy selections because policy-specific rules are applied inside the function body.
     */
} //namespace base::vocab::inline ptr

/**
 * @brief Partial specialization of `std::pointer_traits` for `VocabPtr`s.
 *
 * @tparam T The concrete pointer specialization.
 */
template<base::vocab::ptr::VocabPtr T>
struct std::pointer_traits<T> {
    using pointer = T;
    using element_type = typename pointer::element_type;
    using difference_type = typename pointer::difference_type;

    template<class OtherPointee>
    using rebind = typename pointer::template rebind<OtherPointee>;

    ///@brief Produces a pointer instanc bound to a given object.
    static constexpr pointer pointer_to(typename pointer::reference object) noexcept(noexcept(pointer::pointer_to(object))) requires (!std::is_void_v<element_type>) { return pointer::pointer_to(object); }

    ///@brief Returns the pointer's stored address.
    static constexpr typename pointer::address_type to_address(pointer ptr) noexcept(noexcept(ptr.get())) { return ptr.get(); }
}; //struct std::pointer_traits

/**
 * @brief Partial specialization of `std::hash` for `VocabPtr`s.
 *
 * @tparam T The concrete pointer specialization.
 *
 * @remark Hashes the stored address rather than pointee object state or values.
 * @remark Consistent with `ptr_core` equality semantics.
 */
template<base::vocab::ptr::VocabPtr T>
struct std::hash<T> {
    ///@brief Hashes the pointer based on the underlying address.
    [[nodiscard]] constexpr std::size_t operator()(const T& ptr) const noexcept
    {
        return std::hash<typename T::address_type>{}(ptr.get());
    }
}; //struct std::hash

/**
 * @brief Partial specialization of `std::formatter` for `VocabPtr`s.
 *
 * @tparam T The concrete pointer specialization.
 * @tparam CharT The character type used by the format string.
 *
 * @remark Formats the stored address according to the rules for its nested `address_type` type.
 */
template<base::vocab::ptr::VocabPtr T, typename CharT>
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
}; //struct std::formatter

/**
 * @brief Partial specialization of `std::basic_common_reference` for two specializations of the same concrete pointer template with different pointee types.
 *
 * This customization point establishes that the common reference between `ConcretePtr<T>`
 * and `ConcretePtr<U>` is a specialization of that same `ConcretePtr` template, whose
 * pointee type is the common reference of their pointee types.
 *
 * @tparam ConcretePtr The concrete pointer template.
 * @tparam T The pointee type of the left-hand pointer.
 * @tparam U The pointee type of the right-hand pointer.
 * @tparam TQual An internal standard library alias template applying the qualifiers of the left-hand argument.
 * @tparam UQual An internal standard library alias template applying the qualifiers of the right-hand argument.
 *
 * @remark Determines the common reference pointee exactly like raw pointers do when finding their common reference.
 */
template<
    template<typename> typename ConcretePtr,
    typename T,
    typename U,
    template<typename> typename TQual,
    template<typename> typename UQual
>
    requires (!std::same_as<T, U>) && base::vocab::ptr::VocabPtr<ConcretePtr<T>> && base::vocab::ptr::VocabPtr<ConcretePtr<U>>
          && std::common_reference_with<TQual<T*>, UQual<U*>>
struct std::basic_common_reference<ConcretePtr<T>, ConcretePtr<U>, TQual, UQual> {
private:
    using raw_common_ref = std::common_reference_t<TQual<T*>, UQual<U*>>;

public:
    using type = base::meta::traits::
        copy_cvref_t<raw_common_ref, ConcretePtr<std::remove_pointer_t<std::remove_cvref_t<raw_common_ref>>>>;
}; //struct std::basic_common_reference

/**
 * @brief Partial specialization of `std::basic_common_reference` for two distinct `VocabPtr` types.
 *
 * This customization point resolves the common reference between two specializations of different
 * vocabulary pointers to the common reference of their nested `address_type`s.
 *
 * @tparam T The first concrete pointer specialization.
 * @tparam U The second concrete pointer specialization.
 * @tparam TQual An internal standard library alias template applying the cv/ref qualifiers of `T`.
 * @tparam UQual An internal standard library alias template applying the cv/ref qualifiers of `U`.
 */
template<
    base::vocab::ptr::VocabPtr T,
    base::vocab::ptr::VocabPtr U,
    template<typename> typename TQual,
    template<typename> typename UQual
>
    requires (!std::same_as<T, U>)
          && std::common_reference_with<TQual<typename T::address_type>, UQual<typename U::address_type>>
struct std::basic_common_reference<T, U, TQual, UQual> {
    using type = std::common_reference_t<TQual<typename T::address_type>, UQual<typename U::address_type>>;
}; //struct std::basic_common_reference

/**
 * @brief Partial specialization of `std::basic_common_reference` for `VocabPtr`s with raw pointers.
 *
 * This customization point establishes that the common reference domain between a `VocabPtr` and a
 * raw pointer is equivalent to the common reference of their respective raw address types.
 *
 * @tparam T The concrete pointer specialization.
 * @tparam OtherPointee The raw pointer's pointee type whose address shares a common reference with `T::address_type`.
 * @tparam TQual An internal standard library alias template applying the cv/ref qualifiers of `T`.
 * @tparam OtherQual An internal standard library alias template applying the cv/ref qualifiers of `OtherPointee*`.
 */
template<
    base::vocab::ptr::VocabPtr T,
    typename OtherPointee,
    template<typename> typename TQual,
    template<typename> typename OtherQual
>
    requires std::common_reference_with<TQual<typename T::address_type>, OtherQual<OtherPointee*>>
struct std::basic_common_reference<T, OtherPointee*, TQual, OtherQual> {
    using type = std::common_reference_t<TQual<typename T::address_type>, OtherQual<OtherPointee*>>;
}; //struct std::basic_common_reference

/**
 * @brief Partial specialization of `std::basic_common_reference` for raw pointers with `VocabPtr`s.
 *
 * This customization point establishes that the common reference domain between a raw pointer and
 * a `VocabPtr` and a is equivalent to the common reference of their respective raw address types.
 *
 * @tparam T The concrete pointer specialization.
 * @tparam OtherPointee The raw pointer's pointee type whose address shares a common reference with `T::address_type`.
 * @tparam TQual An internal standard library alias template applying the cv/ref qualifiers of `T`.
 * @tparam OtherQual An internal standard library alias template applying the cv/ref qualifiers of `OtherPointee*`.
 */
template<
    base::vocab::ptr::VocabPtr T,
    typename OtherPointee,
    template<typename> typename TQual,
    template<typename> typename OtherQual
>
    requires std::common_reference_with<OtherQual<OtherPointee*>, TQual<typename T::address_type>>
struct std::basic_common_reference<OtherPointee*, T, OtherQual, TQual> {
    using type = std::common_reference_t<OtherQual<OtherPointee*>, TQual<typename T::address_type>>;
}; //struct std::basic_common_reference
