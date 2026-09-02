// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import ut;
import std;

import base.meta.concepts;

using namespace ut;

namespace {
    template<template<typename> typename Ptr>
    struct pointer_test_traits_base;

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::dependency_ptr> {
        static constexpr bool is_nullable              = false;
        static constexpr bool has_arithmetic_traversal = false;
        static constexpr bool allows_pointer_binding   = false;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::required_ptr> {
        static constexpr bool is_nullable              = false;
        static constexpr bool has_arithmetic_traversal = false;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::alias_ptr> {
        static constexpr bool is_nullable              = true;
        static constexpr bool has_arithmetic_traversal = false;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::cursor_ptr> {
        static constexpr bool is_nullable              = false;
        static constexpr bool has_arithmetic_traversal = true;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::iterator_ptr> {
        static constexpr bool is_nullable              = true;
        static constexpr bool has_arithmetic_traversal = true;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<template<typename> typename Ptr>
    struct pointer_test_traits : pointer_test_traits_base<Ptr> {
        static constexpr bool permits_void_pointee = !pointer_test_traits_base<Ptr>::has_arithmetic_traversal
                                                  && pointer_test_traits_base<Ptr>::allows_pointer_binding;
    };

    template<typename Lambda>
    //NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): Forwarding is not needed to call the lambda.
    constexpr void test_each_pointer_type_with(Lambda&& test_impl)
    {
        test_impl.template operator()<base::vocab::ptr::dependency_ptr>();
        test_impl.template operator()<base::vocab::ptr::required_ptr>();
        test_impl.template operator()<base::vocab::ptr::alias_ptr>();
        test_impl.template operator()<base::vocab::ptr::cursor_ptr>();
        test_impl.template operator()<base::vocab::ptr::iterator_ptr>();
    }

    template<typename T>
    concept has_addition = requires(T t) { t + 1; } || requires(T t) { 1 + t; };

    template<typename T>
    concept has_subtraction = requires(T t) { t - 1; };

    template<typename T>
    concept has_difference = requires(T t) { t - t; };

    template<typename T>
    concept has_pre_increment = requires(T t) { ++t; };

    template<typename T>
    concept has_post_increment = requires(T t) { t++; };

    template<typename T>
    concept has_pre_decrement = requires(T t) { --t; };

    template<typename T>
    concept has_post_decrement = requires(T t) { t--; };

    template<typename T>
    concept dereferenceable = requires(T t) { *t; };

    template<typename T>
    concept arrow_accessible = requires(T t) { t.operator->(); };

    template<typename T>
    concept has_pointer_to = requires(T::element_type obj) { T::pointer_to(obj); };

    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct mixin_1 {
        virtual ~mixin_1() = default;
    };

    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct mixin_2 {
        virtual ~mixin_2() = default;
    };

    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct base_type : mixin_1,
                       mixin_2 {
        ~base_type() override = default;
        std::int32_t value{0};
    };

    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct derived_type : base_type {
        ~derived_type() override = default;
        std::int32_t extra{42};
    };

    union union_type {
        std::int32_t value;
        std::int16_t irrelevant;
    };

    template<typename T>
    struct trivial_smart_ptr {
        T* address{};

        [[nodiscard]] T* get() const { return address; }

        T* operator->() const { return address; }
    };

    struct ref_tag;
    struct ptr_tag;
    struct smart_ptr_tag;

    template<typename T, typename Tag>
    struct source_category;

    template<typename T>
    struct source_category<T, ref_tag> {
        using type = std::add_lvalue_reference_t<T>;
    };

    template<typename T>
    struct source_category<T, ptr_tag> {
        using type = std::add_pointer_t<T>;
    };

    template<typename T>
    struct source_category<T, smart_ptr_tag> {
        using type = trivial_smart_ptr<T>&;
    };

    template<typename T, typename Tag>
    using source_t = source_category<T, Tag>::type;

    //NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity): Test framework.
    suite concrete_pointer_parameterized_tests = [] mutable {
        //============================================================
        // Template Constraint Validation
        //============================================================

        "template instantiation checks"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using base::meta::concepts::instantiable_with;
                expect(eq(instantiable_with<ConcretePtr, std::int32_t>, true));
                expect(eq(instantiable_with<ConcretePtr, std::int32_t*>, true));
                expect(eq(instantiable_with<ConcretePtr, std::map<std::string, std::vector<std::int32_t>>>, true));

                expect(eq(instantiable_with<ConcretePtr, void>, pointer_test_traits<ConcretePtr>::permits_void_pointee));

                expect(eq(instantiable_with<ConcretePtr, std::int32_t&>, false));
                expect(eq(instantiable_with<ConcretePtr, std::int32_t&&>, false));
                expect(eq(instantiable_with<ConcretePtr, void(std::int32_t)>, false));
                expect(eq(instantiable_with<ConcretePtr, void (&)(std::int32_t)>, false));
                expect(eq(instantiable_with<ConcretePtr, void (*)(std::int32_t, float)>, false));
                expect(eq(instantiable_with<ConcretePtr, void (**)(std::string, std::int32_t)>, false));
                expect(eq(instantiable_with<ConcretePtr, void (*******)(std::int32_t)>, false));
            });
        };

        //============================================================
        // Triviality & ABI properties
        //============================================================

        "triviality"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const auto test_impl = []<typename Pointee> {
                    expect(eq(std::is_standard_layout_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_copyable_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_destructible_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_copy_constructible_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_move_constructible_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_copy_assignable_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_move_assignable_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_nothrow_constructible_v<ConcretePtr<Pointee>, Pointee&>, true));
                    expect(eq(std::is_nothrow_swappable_v<ConcretePtr<Pointee>>, true));
                };

                test_impl.template operator()<std::int32_t>();
                test_impl.template operator()<std::map<std::string, std::vector<std::int32_t>>>();
            });
        };

        "size and alignment match raw pointers"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using simple_t = std::int32_t;

                expect(eq(sizeof(ConcretePtr<simple_t>), sizeof(simple_t*)));
                expect(eq(alignof(ConcretePtr<simple_t>), alignof(simple_t*)));

                using complex_t = std::map<std::string, std::vector<std::int32_t>>;

                expect(eq(sizeof(ConcretePtr<complex_t>), sizeof(complex_t*)));
                expect(eq(alignof(ConcretePtr<complex_t>), alignof(complex_t*)));
            });
        };

        //============================================================
        // Type properties
        //============================================================

        "type aliases are correct"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using t      = ConcretePtr<const std::int32_t>;
                using traits = std::pointer_traits<t>;

                constexpr bool t_pointer = std::same_as<typename traits::pointer, t>;
                constexpr bool element   = std::same_as<typename t::element_type, const std::int32_t>;
                constexpr bool t_element = std::same_as<typename traits::element_type, typename t::element_type>;
                constexpr bool value     = std::same_as<typename t::value_type, std::int32_t>;
                constexpr bool address   = std::same_as<typename t::address_type, const std::int32_t*>;
                constexpr bool lref      = std::same_as<typename t::reference, const std::int32_t&>;
                constexpr bool rref      = std::same_as<typename t::rvalue_reference, const std::int32_t&&>;
                constexpr bool ptrdiff   = std::same_as<typename t::difference_type, std::ptrdiff_t>;
                constexpr bool t_ptrdiff = std::same_as<typename traits::difference_type, typename t::difference_type>;

                expect(eq(t_pointer, true));
                expect(eq(element, true));
                expect(eq(t_element, true));
                expect(eq(value, true));
                expect(eq(address, true));
                expect(eq(lref, true));
                expect(eq(rref, true));
                expect(eq(ptrdiff, true));
                expect(eq(t_ptrdiff, true));
            });
        };

        "`rebind` metafunctions preserve the pointer template while changing the pointee type"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using pointee1 = std::int32_t;
                using pointee2 = const std::map<std::string, std::vector<std::int32_t>>;

                expect(
                    eq(std::same_as<typename ConcretePtr<pointee1>::template rebind<pointee2>, ConcretePtr<pointee2>>, true)
                );
                expect(
                    eq(std::same_as<
                           typename std::pointer_traits<ConcretePtr<pointee1>>::template rebind<pointee2>,
                           ConcretePtr<pointee2>
                       >,
                       true)
                );
            });
        };

        //============================================================
        // Construction (Initial Binding) / Assignment (Rebinding)
        //============================================================

        "bindable from nullptr according to nullability policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(
                    eq(std::constructible_from<ConcretePtr<std::int32_t>, std::nullptr_t>,
                       pointer_test_traits<ConcretePtr>::is_nullable)
                );
                expect(
                    eq(std::is_assignable_v<ConcretePtr<std::int32_t>&, std::nullptr_t>,
                       pointer_test_traits<ConcretePtr>::is_nullable)
                );
            });
        };

        "pointer binding according to policies"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const auto
                    verify_binding_operations = [] < typename Pointee,
                    typename SourceTag, bool IsConstructibleFrom, bool IsConvertibleFrom,
                    bool IsAssignableFrom = IsConstructibleFrom && !std::same_as<SourceTag, ref_tag> > {
                        //Explicitly constructible unless removing qualifier
                        expect(
                            eq(std::constructible_from<ConcretePtr<Pointee>, source_t<Pointee, SourceTag>>, IsConstructibleFrom)
                        );
                        expect(eq(std::constructible_from<ConcretePtr<Pointee>, source_t<const Pointee, SourceTag>>, false));
                        expect(eq(std::constructible_from<ConcretePtr<Pointee>, source_t<volatile Pointee, SourceTag>>, false));
                        expect(
                            eq(std::constructible_from<ConcretePtr<Pointee>, source_t<const volatile Pointee, SourceTag>>,
                               false)
                        );
                        expect(
                            eq(std::constructible_from<ConcretePtr<const Pointee>, source_t<Pointee, SourceTag>>,
                               IsConstructibleFrom)
                        );
                        expect(
                            eq(std::constructible_from<ConcretePtr<const Pointee>, source_t<const Pointee, SourceTag>>,
                               IsConstructibleFrom)
                        );
                        expect(
                            eq(std::constructible_from<ConcretePtr<const Pointee>, source_t<volatile Pointee, SourceTag>>,
                               false)
                        );
                        expect(
                            eq(std::constructible_from<ConcretePtr<const Pointee>, source_t<const volatile Pointee, SourceTag>>,
                               false)
                        );
                        expect(
                            eq(std::constructible_from<ConcretePtr<volatile Pointee>, source_t<Pointee, SourceTag>>,
                               IsConstructibleFrom)
                        );
                        expect(
                            eq(std::constructible_from<ConcretePtr<volatile Pointee>, source_t<const Pointee, SourceTag>>,
                               false)
                        );
                        expect(
                            eq(std::constructible_from<ConcretePtr<volatile Pointee>, source_t<volatile Pointee, SourceTag>>,
                               IsConstructibleFrom)
                        );
                        expect(eq(
                            std::constructible_from<ConcretePtr<volatile Pointee>, source_t<const volatile Pointee, SourceTag>>,
                            false
                        ));
                        expect(
                            eq(std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<Pointee, SourceTag>>,
                               IsConstructibleFrom)
                        );
                        expect(
                            eq(std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<const Pointee, SourceTag>>,
                               IsConstructibleFrom)
                        );
                        expect(eq(
                            std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<volatile Pointee, SourceTag>>,
                            IsConstructibleFrom
                        ));
                        expect(
                            eq(std::constructible_from<
                                   ConcretePtr<const volatile Pointee>,
                                   source_t<const volatile Pointee, SourceTag>
                               >,
                               IsConstructibleFrom)
                        );

                        //Implicitly convertible unless removing qualifier
                        expect(eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<Pointee>>, IsConvertibleFrom));
                        expect(eq(std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<Pointee>>, false));
                        expect(eq(std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<Pointee>>, false));
                        expect(
                            eq(std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<Pointee>>, false)
                        );
                        expect(
                            eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<const Pointee>>, IsConvertibleFrom)
                        );
                        expect(
                            eq(std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<const Pointee>>,
                               IsConvertibleFrom)
                        );
                        expect(
                            eq(std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<const Pointee>>, false)
                        );
                        expect(
                            eq(std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<const Pointee>>,
                               false)
                        );
                        expect(
                            eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<volatile Pointee>>,
                               IsConvertibleFrom)
                        );
                        expect(
                            eq(std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<volatile Pointee>>, false)
                        );
                        expect(
                            eq(std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<volatile Pointee>>,
                               IsConvertibleFrom)
                        );
                        expect(
                            eq(std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<volatile Pointee>>,
                               false)
                        );
                        expect(
                            eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>,
                               IsConvertibleFrom)
                        );
                        expect(
                            eq(std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>,
                               IsConvertibleFrom)
                        );
                        expect(
                            eq(std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>,
                               IsConvertibleFrom)
                        );
                        expect(
                            eq(std::convertible_to<
                                   source_t<const volatile Pointee, SourceTag>,
                                   ConcretePtr<const volatile Pointee>
                               >,
                               IsConvertibleFrom)
                        );

                        //Assignable unless removing qualifier or rebinding from reference
                        expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, source_t<Pointee, SourceTag>>, IsAssignableFrom));
                        expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, source_t<const Pointee, SourceTag>>, false));
                        expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, source_t<volatile Pointee, SourceTag>>, false));
                        expect(
                            eq(std::is_assignable_v<ConcretePtr<Pointee>&, source_t<const volatile Pointee, SourceTag>>, false)
                        );
                        expect(
                            eq(std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<Pointee, SourceTag>>,
                               IsAssignableFrom)
                        );
                        expect(
                            eq(std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<const Pointee, SourceTag>>,
                               IsAssignableFrom)
                        );
                        expect(
                            eq(std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<volatile Pointee, SourceTag>>, false)
                        );
                        expect(
                            eq(std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<const volatile Pointee, SourceTag>>,
                               false)
                        );
                        expect(
                            eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<Pointee, SourceTag>>,
                               IsAssignableFrom)
                        );
                        expect(
                            eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<const Pointee, SourceTag>>, false)
                        );
                        expect(
                            eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<volatile Pointee, SourceTag>>,
                               IsAssignableFrom)
                        );
                        expect(eq(
                            std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<const volatile Pointee, SourceTag>>,
                            false
                        ));
                        expect(
                            eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<Pointee, SourceTag>>,
                               IsAssignableFrom)
                        );
                        expect(
                            eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<const Pointee, SourceTag>>,
                               IsAssignableFrom)
                        );
                        expect(eq(
                            std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<volatile Pointee, SourceTag>>,
                            IsAssignableFrom
                        ));
                        expect(
                            eq(std::is_assignable_v<
                                   ConcretePtr<const volatile Pointee>&,
                                   source_t<const volatile Pointee, SourceTag>
                               >,
                               IsAssignableFrom)
                        );
                    };

                //Reference binding is explicit when allowed
                verify_binding_operations.template
                    operator()<std::int32_t, ref_tag, pointer_test_traits<ConcretePtr>::allows_reference_binding, false>();

                //Pointer binding allows implicit conversion
                verify_binding_operations.template operator()<
                    std::int32_t,
                    ptr_tag,
                    pointer_test_traits<ConcretePtr>::allows_pointer_binding,
                    pointer_test_traits<ConcretePtr>::allows_pointer_binding
                >();
                verify_binding_operations.template operator()<
                    std::int32_t,
                    smart_ptr_tag,
                    pointer_test_traits<ConcretePtr>::allows_pointer_binding,
                    pointer_test_traits<ConcretePtr>::allows_pointer_binding
                >();
            });
        };

        "not bindable from pointee rvalue"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, std::int32_t>, false));
                expect(eq(std::constructible_from<ConcretePtr<const std::int32_t>, std::int32_t>, false));
                expect(eq(std::constructible_from<ConcretePtr<const std::int32_t>, const std::int32_t>, false));

                expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, std::int32_t&&>, false));
                expect(eq(std::constructible_from<ConcretePtr<const std::int32_t>, std::int32_t&&>, false));
                expect(eq(std::constructible_from<ConcretePtr<const std::int32_t>, const std::int32_t&&>, false));

                expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t>&, std::int32_t&&>, false));
                expect(eq(std::is_assignable_v<ConcretePtr<const std::int32_t>&, std::int32_t&&>, false));
                expect(eq(std::is_assignable_v<ConcretePtr<const std::int32_t>&, const std::int32_t&&>, false));
            });
        };

        "not bindable from smart pointer rvalue"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, trivial_smart_ptr<std::int32_t>>, false));
                expect(eq(std::constructible_from<ConcretePtr<const std::int32_t>, trivial_smart_ptr<std::int32_t>>, false));
                expect(
                    eq(std::constructible_from<ConcretePtr<const std::int32_t>, const trivial_smart_ptr<std::int32_t>>, false)
                );

                expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, trivial_smart_ptr<std::int32_t>&&>, false));
                expect(eq(std::constructible_from<ConcretePtr<const std::int32_t>, trivial_smart_ptr<std::int32_t>&&>, false));
                expect(
                    eq(std::constructible_from<ConcretePtr<const std::int32_t>, const trivial_smart_ptr<std::int32_t>&&>, false)
                );

                expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t>&, trivial_smart_ptr<std::int32_t>&&>, false));
                expect(eq(std::is_assignable_v<ConcretePtr<const std::int32_t>&, trivial_smart_ptr<std::int32_t>&&>, false));
                expect(
                    eq(std::is_assignable_v<ConcretePtr<const std::int32_t>&, const trivial_smart_ptr<std::int32_t>&&>, false)
                );
            });
        };

        //============================================================
        // Pointer semantics
        //============================================================

        "`pointer_to` forms a valid pointer instance whose `get` returns its stored address"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using pointee_t = std::int32_t;
                using pointer_t = const ConcretePtr<pointee_t>;

                pointee_t obj{};

                const auto class_ptr = pointer_t::pointer_to(obj);
                const auto trait_ptr = std::pointer_traits<pointer_t>::pointer_to(obj);
                const auto free_ptr  = base::vocab::ptr::pointer_to<ConcretePtr>(obj);

                expect(eq(std::same_as<decltype(class_ptr), pointer_t>, true));
                expect(eq(std::same_as<decltype(trait_ptr), pointer_t>, true));
                expect(eq(std::same_as<decltype(free_ptr), pointer_t>, true));

                expect(eq(class_ptr.get(), std::addressof(obj)));
                expect(eq(trait_ptr.get(), std::addressof(obj)));
                expect(eq(free_ptr.get(), std::addressof(obj)));
            });
        };

        "`to_address` returns stored address as raw pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using pointee_t = const std::int32_t;
                using pointer_t = ConcretePtr<pointee_t>;

                pointee_t obj{};

                const auto ptr = pointer_t::pointer_to(obj);

                expect(eq(
                    std::same_as<decltype(std::pointer_traits<pointer_t>::to_address(ptr)), typename pointer_t::address_type>,
                    true
                ));
                expect(eq(std::same_as<decltype(std::to_address(ptr)), typename pointer_t::address_type>, true));

                expect(eq(std::pointer_traits<pointer_t>::to_address(ptr), std::addressof(obj)));
                expect(eq(std::to_address(ptr), std::addressof(obj)));
            });
        };

        "operator* dereferences correctly"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const auto value = 55;
                const auto ptr   = base::vocab::pointer_to<ConcretePtr>(value);

                expect(eq(*ptr, value));
            });
        };

        //NOLINTBEGIN(readability-magic-numbers): Test fixture needs a meaningless number.
        //NOLINTBEGIN(cppcoreguidelines-pro-type-union-access): Testing union access.
        "operator-> provides member access"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(eq(ArrowAccessible<ConcretePtr<std::uint8_t>>, false));
                expect(eq(ArrowAccessible<ConcretePtr<std::float_round_style>>, false));
                expect(eq(ArrowAccessible<ConcretePtr<std::memory_order>>, false));
                expect(eq(ArrowAccessible<ConcretePtr<std::byte>>, false));
                expect(eq(ArrowAccessible<ConcretePtr<base_type>>, true));
                expect(eq(ArrowAccessible<ConcretePtr<union_type>>, true));

                base_type c_obj;
                c_obj.value     = 123;
                const auto ptr1 = base::vocab::pointer_to<ConcretePtr>(c_obj);

                union_type u_obj{};
                u_obj.value     = 321;
                const auto ptr2 = base::vocab::pointer_to<ConcretePtr>(u_obj);

                expect(eq(ptr1->value, c_obj.value));
                expect(eq(ptr2->value, u_obj.value));
            });
        };
        //NOLINTEND(cppcoreguidelines-pro-type-union-access)
        //NOLINTEND(readability-magic-numbers)

        "implicit conversion to raw pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using test_type = std::int32_t;

                expect(eq(std::convertible_to<ConcretePtr<test_type>, test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<test_type>, const test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<test_type>, volatile test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<test_type>, const volatile test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<const test_type>, test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<const test_type>, const test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<const test_type>, volatile test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<const test_type>, const volatile test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<volatile test_type>, test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<volatile test_type>, const test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<volatile test_type>, volatile test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<volatile test_type>, const volatile test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<const volatile test_type>, test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<const volatile test_type>, const test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<const volatile test_type>, volatile test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<const volatile test_type>, const volatile test_type*>, true));
            });
        };

        "conversion to raw pointer preserves address"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const std::int32_t value{};
                const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                const std::int32_t* raw = ptr;

                expect(eq(raw, std::addressof(value)));
            });
        };

        "contextual boolean conversion is supported"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(eq(std::constructible_from<bool, ConcretePtr<std::int32_t>>, true));

                const std::int32_t value{};
                const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                bool converted{false};
                if (ptr) {
                    converted = true;
                }

                expect(eq(static_cast<bool>(ptr), true));
                expect(eq(!ptr, false));
                expect(eq(converted, true));
            });
        };

        //============================================================
        // Rebinding
        //============================================================

        "rebind via copy-assignment from reference construction"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    const std::int32_t value1{};
                    const std::int32_t value2 = 2;

                    ConcretePtr<const std::int32_t> ptr{value1};
                    //error: ```ptr = value2;``` is deleted to prevent implicit conversions
                    ptr = ConcretePtr{value2};

                    expect(eq(*ptr, value2));
                    expect(eq(ptr.get(), std::addressof(value2)));
                }
            });
        };

        "rebind via `reset` call with reference argument"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    const std::int32_t value1{};
                    const std::int32_t value2 = 2;

                    ConcretePtr<const std::int32_t> ptr{value1};
                    ptr.reset(value2);

                    expect(eq(*ptr, value2));
                    expect(eq(ptr.get(), std::addressof(value2)));
                }
            });
        };

        "moved-from object may be rebound"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                std::int32_t value1 = 1;
                std::int32_t value2 = 2;

                auto source = base::vocab::pointer_to<ConcretePtr>(value1);
                ConcretePtr<std::int32_t> target{std::move(source)};

                //Rebind moved-from `source` to reference `value2`
                source = base::vocab::pointer_to<ConcretePtr>(value2);

                expect(eq(*target, value1));
                expect(eq(*source, value2));
            });
        };

        "rebind via `reset()` disengages the pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    const std::int32_t value{};

                    auto ptr = base::vocab::pointer_to<ConcretePtr>(value);
                    ptr.reset();

                    expect(eq(!ptr, true));
                }
            });
        };

        //============================================================
        // Nullability-based exception throwing
        //============================================================

        "constructing from null raw pointer throws according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    const std::int32_t value{42};
                    const std::int32_t* const bound_source{std::addressof(value)};
                    const std::int32_t* const null_source{};

                    bool threw_when_bound = false;
                    try {
                        const ConcretePtr<const std::int32_t> ptr{bound_source};

                        expect(eq(*ptr, value));
                        expect(eq(ptr.get(), bound_source));
                    } catch (...) {
                        threw_when_bound = true;
                    }

                    expect(eq(threw_when_bound, false));

                    bool threw_when_null = false;
                    bool wrong_exception = false;
                    try {
                        [[maybe_unused]] const ConcretePtr<const std::int32_t> dummy_ptr{null_source};
                    } catch (const std::invalid_argument&) {
                        threw_when_null = true;
                    } catch (...) {
                        wrong_exception = true;
                    }

                    expect(eq(threw_when_null, !pointer_test_traits<ConcretePtr>::is_nullable));
                    expect(eq(wrong_exception, false));
                }
            });
        };

        "constructing from null smart pointer throws according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    const std::int32_t value{42};
                    const trivial_smart_ptr<const std::int32_t> bound_source{std::addressof(value)};
                    const trivial_smart_ptr<const std::int32_t> null_source{};

                    bool threw_when_bound = false;
                    try {
                        const ConcretePtr<const std::int32_t> ptr{bound_source};

                        expect(eq(*ptr, value));
                        expect(eq(ptr.get(), bound_source.get()));
                    } catch (...) {
                        threw_when_bound = true;
                    }

                    expect(eq(threw_when_bound, false));

                    bool threw_when_null = false;
                    bool wrong_exception = false;
                    try {
                        [[maybe_unused]] const ConcretePtr<const std::int32_t> dummy_ptr{null_source};
                    } catch (const std::invalid_argument&) {
                        threw_when_null = true;
                    } catch (...) {
                        wrong_exception = true;
                    }

                    expect(eq(threw_when_null, !pointer_test_traits<ConcretePtr>::is_nullable));
                    expect(eq(wrong_exception, false));
                }
            });
        };

        "assigning from null raw pointer throws according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    const std::int32_t value{42};
                    const std::int32_t other{};

                    const std::int32_t* const bound_source{std::addressof(value)};
                    const std::int32_t* const null_source{nullptr};

                    ConcretePtr<const std::int32_t> ptr{other};

                    bool threw_when_bound = false;
                    try {
                        ptr = bound_source;
                    } catch (...) {
                        threw_when_bound = true;
                    }

                    expect(eq(threw_when_bound, false));
                    expect(eq(*ptr, *bound_source));
                    expect(eq(ptr.get(), bound_source));

                    bool threw_when_null = false;
                    bool wrong_exception = false;
                    try {
                        ptr = null_source;
                    } catch (const std::invalid_argument&) {
                        threw_when_null = true;
                    } catch (...) {
                        wrong_exception = true;
                    }

                    expect(eq(threw_when_null, !pointer_test_traits<ConcretePtr>::is_nullable));
                    expect(eq(wrong_exception, false));

                    if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                        expect(eq(ptr.get(), null_source));
                    } else {
                        //Invariant preserved after failed assignment
                        expect(eq(*ptr, *bound_source));
                        expect(eq(ptr.get(), bound_source));
                    }
                }
            });
        };

        "assigning from null smart pointer throws according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    const std::int32_t value{42};
                    const std::int32_t other{};

                    const trivial_smart_ptr<const std::int32_t> bound_source{std::addressof(value)};
                    const trivial_smart_ptr<std::int32_t> null_source{};

                    ConcretePtr<const std::int32_t> ptr{other};

                    bool threw_when_bound = false;
                    try {
                        ptr = bound_source;
                    } catch (...) {
                        threw_when_bound = true;
                    }

                    expect(eq(threw_when_bound, false));
                    expect(eq(*ptr, value));
                    expect(eq(ptr.get(), bound_source.get()));

                    bool threw_when_null = false;
                    bool wrong_exception = false;
                    try {
                        ptr = null_source;
                    } catch (const std::invalid_argument&) {
                        threw_when_null = true;
                    } catch (...) {
                        wrong_exception = true;
                    }

                    expect(eq(threw_when_null, !pointer_test_traits<ConcretePtr>::is_nullable));
                    expect(eq(wrong_exception, false));

                    if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                        expect(eq(ptr.get() == null_source.get(), true));
                    } else {
                        //Invariant preserved after failed assignment
                        expect(eq(*ptr, value));
                        expect(eq(ptr.get(), bound_source.get()));
                    }
                }
            });
        };

        //============================================================
        // Swap
        //============================================================

        "swap exchanges bindings"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const std::int32_t value1 = 1;
                const std::int32_t value2 = 2;

                auto lhs = base::vocab::pointer_to<ConcretePtr>(value1);
                auto rhs = base::vocab::pointer_to<ConcretePtr>(value2);

                using std::swap;
                swap(lhs, rhs);

                expect(eq(lhs.get(), std::addressof(value2)));
                expect(eq(rhs.get(), std::addressof(value1)));

                expect(eq(*lhs, value2));
                expect(eq(*rhs, value1));
            });
        };

        //============================================================
        // Equality semantics
        //============================================================

        "equality compares pointer identity"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(eq(std::equality_comparable<ConcretePtr<std::int32_t>>, true));

                std::int32_t x = 1;
                std::int32_t y = 1;

                const auto ptr1 = base::vocab::pointer_to<ConcretePtr, const std::int32_t>(x);
                const auto ptr2 = base::vocab::pointer_to<ConcretePtr>(x);
                const auto ptr3 = base::vocab::pointer_to<ConcretePtr>(y);

                expect(eq(ptr1 == ptr2, true));
                expect(eq(ptr1 == ptr3, false));
                expect(eq(ptr2 == ptr3, false));
            });
        };

        "nullable comparisons with nullptr"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (
                    pointer_test_traits<ConcretePtr>::allows_pointer_binding && pointer_test_traits<ConcretePtr>::is_nullable
                ) {
                    std::int32_t value{};

                    const ConcretePtr<std::int32_t> bound{std::addressof(value)};
                    const ConcretePtr<std::int32_t> null{nullptr};

                    //Expecting both operator== and operator!= to be synthesized correctly
                    expect(eq(bound == nullptr, false));
                    expect(eq(nullptr == bound, false));

                    expect(eq(bound != nullptr, true));
                    expect(eq(nullptr != bound, true));

                    expect(eq(null == nullptr, true));
                    expect(eq(nullptr == null, true));

                    expect(eq(null != nullptr, false));
                    expect(eq(nullptr != null, false));
                }
            });
        };

        "null pointers of same pointer type compare equal"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    const ConcretePtr<std::int32_t> lhs{nullptr};
                    const ConcretePtr<const std::int32_t> rhs{nullptr};

                    expect(eq(lhs == rhs, true));
                    expect(eq(lhs != rhs, false));
                }
            });
        };

        //============================================================
        // Covariance
        //============================================================

        "construct base from derived pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                derived_type d_obj;
                const auto d_ptr = base::vocab::pointer_to<ConcretePtr>(d_obj);

                const ConcretePtr<base_type> b_ptr{d_ptr};

                expect(eq(b_ptr.get(), static_cast<base_type*>(std::addressof(d_obj))));
            });
        };

        "construct base from derived reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                derived_type d_obj;

                const auto b_ptr = base::vocab::pointer_to<ConcretePtr, base_type>(d_obj);

                expect(eq(b_ptr.get(), static_cast<base_type*>(std::addressof(d_obj))));
            });
        };

        "assign base from derived pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                derived_type d_obj;
                const auto d_ptr = base::vocab::pointer_to<ConcretePtr>(d_obj);

                base_type b_obj;
                auto b_ptr = base::vocab::pointer_to<ConcretePtr>(b_obj);

                b_ptr = d_ptr;

                expect(eq(b_ptr.get(), static_cast<base_type*>(std::addressof(d_obj))));
            });
        };

        "rebind base from derived reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    derived_type d_obj;

                    base_type b_obj;
                    ConcretePtr<base_type> b_ptr{b_obj};

                    b_ptr = ConcretePtr{d_obj};

                    expect(eq(b_ptr.get(), static_cast<base_type*>(std::addressof(d_obj))));
                }
            });
        };

        "assign const base from derived pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                derived_type d_obj;
                const auto d_ptr = base::vocab::pointer_to<ConcretePtr>(d_obj);

                const base_type b_obj;
                auto b_ptr = base::vocab::pointer_to<ConcretePtr, const base_type>(b_obj);

                b_ptr = d_ptr;

                expect(eq(b_ptr.get(), static_cast<const base_type*>(std::addressof(d_obj))));
            });
        };

        "rebind const base from derived reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    derived_type d_obj;

                    const base_type b_obj;
                    ConcretePtr<const base_type> b_ptr{b_obj};

                    b_ptr = ConcretePtr{d_obj};

                    expect(eq(b_ptr.get(), static_cast<const base_type*>(std::addressof(d_obj))));
                }
            });
        };

        "covariant equality comparison"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(eq(std::equality_comparable_with<ConcretePtr<base_type>, ConcretePtr<derived_type>>, true));
                expect(eq(std::equality_comparable_with<ConcretePtr<base_type>, derived_type*>, true));
                expect(eq(std::equality_comparable_with<base_type*, ConcretePtr<derived_type>>, true));
            });
        };

        //============================================================
        // Pointer Casting & Lifetime Transmutation
        //============================================================
        //NOLINTBEGIN(misc-const-correctness): Readability suffers with const correctness in these tests.
        "const_pointer_cast alters pointee cv-qualifications"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const auto test_cast = []<typename Source, typename Destination> {
                    Source value{};

                    auto source = base::vocab::pointer_to<ConcretePtr>(value);
                    auto result = const_pointer_cast<Destination>(source);

                    expect(eq(std::same_as<decltype(result), ConcretePtr<Destination>>, true));
                    expect(eq(result.get() == std::addressof(value), true));
                };

                test_cast.template operator()<std::int32_t, std::int32_t>();
                test_cast.template operator()<std::int32_t, const std::int32_t>();
                test_cast.template operator()<std::int32_t, volatile std::int32_t>();
                test_cast.template operator()<std::int32_t, const volatile std::int32_t>();
                test_cast.template operator()<const std::int32_t, std::int32_t>();
                test_cast.template operator()<const std::int32_t, const std::int32_t>();
                test_cast.template operator()<const std::int32_t, volatile std::int32_t>();
                test_cast.template operator()<const std::int32_t, const volatile std::int32_t>();
                test_cast.template operator()<volatile std::int32_t, std::int32_t>();
                test_cast.template operator()<volatile std::int32_t, const std::int32_t>();
                test_cast.template operator()<volatile std::int32_t, volatile std::int32_t>();
                test_cast.template operator()<volatile std::int32_t, const volatile std::int32_t>();
                test_cast.template operator()<const volatile std::int32_t, std::int32_t>();
                test_cast.template operator()<const volatile std::int32_t, const std::int32_t>();
                test_cast.template operator()<const volatile std::int32_t, volatile std::int32_t>();
                test_cast.template operator()<const volatile std::int32_t, const volatile std::int32_t>();
            });
        };

        "const_pointer_cast preserves null state"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    ConcretePtr<std::int32_t> source{nullptr};

                    const auto result = const_pointer_cast<const std::int32_t>(source);

                    expect(eq(result == nullptr, true));
                }
            });
        };

        "static_pointer_cast converts static pointee type up and down inheritance hierarchies"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                derived_type object;

                ConcretePtr<derived_type> source = base::vocab::pointer_to<ConcretePtr>(object);
                auto result1                     = static_pointer_cast<base_type>(source);

                expect(eq(std::same_as<decltype(result1), ConcretePtr<base_type>>, true));
                expect(eq(result1.get(), static_cast<base_type*>(std::addressof(object))));

                auto result2 = static_pointer_cast<derived_type>(result1);

                expect(eq(std::same_as<decltype(result2), ConcretePtr<derived_type>>, true));
                expect(eq(result2.get(), std::addressof(object)));
            });
        };

        "static_pointer_cast preserves cv-qualifications"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const derived_type object;

                ConcretePtr<const derived_type> source = base::vocab::pointer_to<ConcretePtr>(object);
                auto result                            = static_pointer_cast<base_type>(source);

                expect(eq(std::same_as<decltype(result), ConcretePtr<const base_type>>, true));
            });
        };

        "static_pointer_cast preserves null state"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    ConcretePtr<derived_type> source{nullptr};

                    const auto result = static_pointer_cast<base_type>(source);

                    expect(eq(result == nullptr, true));
                }
            });
        };

        "dynamic_pointer_cast performs multiple-inheritance upcasts"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                derived_type value;

                auto source = base::vocab::pointer_to<ConcretePtr>(value);

                auto result_1 = dynamic_pointer_cast<mixin_1>(source);
                auto result_2 = dynamic_pointer_cast<mixin_2>(source);

                expect(eq(std::same_as<decltype(result_1), ConcretePtr<mixin_1>>, true));
                expect(eq(result_1.get(), dynamic_cast<mixin_1*>(std::addressof(value))));

                expect(eq(std::same_as<decltype(result_2), ConcretePtr<mixin_2>>, true));
                expect(eq(result_2.get(), dynamic_cast<mixin_2*>(std::addressof(value))));
            });
        };

        "dynamic_pointer_cast performs successful downcasts"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                derived_type value;

                ConcretePtr<base_type> source = base::vocab::pointer_to<ConcretePtr>(value);

                auto result = dynamic_pointer_cast<derived_type>(source);

                expect(eq(std::same_as<decltype(result), ConcretePtr<derived_type>>, true));
                expect(eq(result.get(), std::addressof(value)));
                expect(eq(result->extra, value.extra));
            });
        };

        "dynamic_pointer_cast handles failed downcasts according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                struct wrong_derived : base_type {};

                derived_type value;

                ConcretePtr<base_type> source = base::vocab::pointer_to<ConcretePtr>(value);

                bool threw           = false;
                bool wrong_exception = false;
                try {
                    const auto result = dynamic_pointer_cast<wrong_derived>(source);

                    expect(eq(result.get() == nullptr, pointer_test_traits<ConcretePtr>::is_nullable));
                } catch (const std::bad_cast&) {
                    threw = true;
                } catch (...) {
                    wrong_exception = true;
                }

                expect(eq(threw, !pointer_test_traits<ConcretePtr>::is_nullable));
                expect(eq(wrong_exception, false));
            });
        };

        "dynamic_pointer_cast preserves cv-qualifications"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const derived_type object;

                ConcretePtr<const derived_type> source = base::vocab::pointer_to<ConcretePtr>(object);
                auto result                            = dynamic_pointer_cast<base_type>(source);

                expect(eq(std::same_as<decltype(result), ConcretePtr<const base_type>>, true));
            });
        };

        "dynamic_pointer_cast preserves null state"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    ConcretePtr<derived_type> source{nullptr};

                    auto result = dynamic_pointer_cast<base_type>(source);

                    expect(eq(result == nullptr, true));
                }
            });
        };

        //NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast): Testing `reinterpret_pointer_cast` in terms of `reinterpret_cast`.
        "reinterpret_pointer_cast views objects as raw bytes"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                struct object {
                    std::int32_t x;
                    double y;
                };

                object value{};

                auto source       = base::vocab::pointer_to<ConcretePtr>(value);
                auto result_bytes = reinterpret_pointer_cast<std::byte>(source);
                auto result_chars = reinterpret_pointer_cast<char>(source);

                expect(eq(std::same_as<decltype(result_bytes), ConcretePtr<std::byte>>, true));
                expect(eq(result_bytes.get(), reinterpret_cast<std::byte*>(std::addressof(value))));

                expect(eq(std::same_as<decltype(result_chars), ConcretePtr<char>>, true));
                expect(eq(result_chars.get(), reinterpret_cast<char*>(std::addressof(value))));
            });
        };

        "reinterpret_pointer_cast alters how a pointer sees its pointee type"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                struct origin_t {
                    std::int32_t foo;
                    char bar;
                    double baz;
                };
                struct target_t {
                    std::int64_t foo;
                    double bar;
                    std::uint8_t baz;
                };

                origin_t value{};

                auto source = base::vocab::pointer_to<ConcretePtr>(value);
                //WARNING: Using this result pointer's stored address potentially invokes undefined behavior.
                auto result = reinterpret_pointer_cast<target_t>(source);

                expect(eq(std::same_as<decltype(result), ConcretePtr<target_t>>, true));
                expect(eq(result.get(), reinterpret_cast<target_t*>(std::addressof(value))));
            });
        };

        "reinterpret_pointer_cast preserves cv-qualifications"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const derived_type object;

                ConcretePtr<const derived_type> source = base::vocab::pointer_to<ConcretePtr>(object);
                auto result                            = reinterpret_pointer_cast<base_type>(source);

                expect(eq(std::same_as<decltype(result), ConcretePtr<const base_type>>, true));
            });
        };

        "reinterpret_pointer_cast preserves null state"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    ConcretePtr<std::int32_t> source{nullptr};

                    auto result = reinterpret_pointer_cast<std::byte>(source);

                    expect(eq(result.get() == nullptr, true));
                }
            });
        };
        //NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

#if defined(__cpp_lib_start_lifetime_as)
        "start_lifetime_as alters pointee type"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>() {
                struct origin_t {
                    std::int32_t foo;
                    std::int32_t bar;
                    std::int32_t baz;
                    double qux;
                };
                struct target_t {
                    std::int32_t x;
                    std::int32_t y;
                    std::int32_t z;
                    double velocity;
                };

                const origin_t value    = {.foo = 1, .bar = 3, .baz = 5, .qux = 2.0};
                const origin_t expected = value;

                auto source = base::vocab::pointer_to<ConcretePtr>(value);
                auto result = start_lifetime_as<target_t>(source);

                expect(eq(std::same_as<decltype(result), ConcretePtr<const target_t>>, true));
                expect(eq(result.get(), reinterpret_cast<target_t*>(std::addressof(value))));
                expect(eq(result->x, expected.foo));
                expect(eq(result->y, expected.bar));
                expect(eq(result->z, expected.baz));
                expect(eq(result->velocity, expected.qux));
            });
        };
#else
    //NOLINTNEXTLINE(clang-diagnostic-#warnings)
    #warning "std::start_lifetime_as not defined. Tests skipped."
#endif
        //NOLINTEND(misc-const-correctness)

        //============================================================
        // Common Reference
        //============================================================

        "basic_common_reference preserves concrete pointer type with cv-qualifications"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(eq(std::common_reference_with<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>, true));
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>,
                           ConcretePtr<const std::int32_t>
                       >,
                       true)
                );

                expect(eq(std::common_reference_with<ConcretePtr<std::int32_t>, ConcretePtr<volatile std::int32_t>>, true));
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<volatile std::int32_t>>,
                           ConcretePtr<volatile std::int32_t>
                       >,
                       true)
                );

                expect(
                    eq(std::common_reference_with<ConcretePtr<const std::int32_t>, ConcretePtr<const volatile std::int32_t>>,
                       true)
                );
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<const std::int32_t>, ConcretePtr<const volatile std::int32_t>>,
                           ConcretePtr<const volatile std::int32_t>
                       >,
                       true)
                );

                expect(
                    eq(std::common_reference_with<ConcretePtr<volatile std::int32_t>, ConcretePtr<const volatile std::int32_t>>,
                       true)
                );
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<volatile std::int32_t>, ConcretePtr<const volatile std::int32_t>>,
                        ConcretePtr<const volatile std::int32_t>
                    >,
                    true
                ));

                expect(
                    eq(std::common_reference_with<ConcretePtr<volatile std::int32_t>, ConcretePtr<const std::int32_t>>, true)
                );
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<volatile std::int32_t>, ConcretePtr<const std::int32_t>>,
                           ConcretePtr<const volatile std::int32_t>
                       >,
                       true)
                );
            });
        };

        "basic_common_reference uses reference-to-pointer value category propagation"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                static_assert(
                    std::same_as<std::common_reference_t<std::int32_t*&, const std::int32_t*&>, const std::int32_t*>,
                    "Sanity check for raw pointer common_reference_t<T*&, const T*&> -> const T*"
                );
                expect(eq(std::common_reference_with<ConcretePtr<std::int32_t>&, ConcretePtr<const std::int32_t>&>, true));
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<std::int32_t>&, ConcretePtr<const std::int32_t>&>,
                           ConcretePtr<const std::int32_t>
                       >,
                       true)
                );

                static_assert(
                    std::same_as<std::common_reference_t<std::int32_t*&&, const std::int32_t*&&>, const std::int32_t*>,
                    "Sanity check for raw pointer common_reference_t<T*&&, const T*&&> -> const T*"
                );
                expect(eq(std::common_reference_with<ConcretePtr<std::int32_t>&&, ConcretePtr<const std::int32_t>&&>, true));
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<std::int32_t>&&, ConcretePtr<const std::int32_t>&&>,
                           ConcretePtr<const std::int32_t>
                       >,
                       true)
                );

                static_assert(
                    std::same_as<std::common_reference_t<const std::int32_t*&, std::int32_t*&&>, const std::int32_t* const&>,
                    "Sanity check for raw pointer common_reference_t<const T*&, T*&&> -> const T* const &"
                );
                expect(eq(std::common_reference_with<ConcretePtr<const std::int32_t>&, ConcretePtr<std::int32_t>&&>, true));
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<const std::int32_t>&, ConcretePtr<std::int32_t>&&>,
                           const ConcretePtr<const std::int32_t>&
                       >,
                       true)
                );

                static_assert(
                    std::same_as<std::common_reference_t<std::int32_t* const&, std::int32_t*&&>, std::int32_t* const&>,
                    "Sanity check for raw pointer common_reference_t<T* const &, T*&&> -> T* const &"
                );
                expect(
                    eq(std::common_reference_with<const ConcretePtr<std::int32_t>&, ConcretePtr<const std::int32_t>&&>, true)
                );
                expect(
                    eq(std::same_as<
                           std::common_reference_t<const ConcretePtr<std::int32_t>&, ConcretePtr<const std::int32_t>&&>,
                           const ConcretePtr<const std::int32_t>&
                       >,
                       true)
                );
            });
        };

        "basic_common_reference matches raw pointer common_reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(
                    eq(std::common_reference_with<ConcretePtr<std::int32_t>, ConcretePtr<const volatile std::int32_t>>, true)
                );
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<const volatile std::int32_t>>,
                           ConcretePtr<std::remove_pointer_t<
                               std::remove_cvref_t<std::common_reference_t<std::int32_t*, const volatile std::int32_t*>>
                           >>
                       >,
                       true)
                );
            });
        };

        "vocabulary pointer and raw pointer share raw pointer common reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(eq(std::common_reference_with<ConcretePtr<std::int32_t>, std::int32_t*>, true));
                expect(
                    eq(std::same_as<std::common_reference_t<ConcretePtr<std::int32_t>, std::int32_t*>, std::int32_t*>, true)
                );

                expect(eq(std::common_reference_with<std::int32_t*, ConcretePtr<std::int32_t>>, true));
                expect(
                    eq(std::same_as<std::common_reference_t<std::int32_t*, ConcretePtr<std::int32_t>>, std::int32_t*>, true)
                );
            });
        };

        "common_reference supports covariance"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(eq(std::common_reference_with<ConcretePtr<derived_type>, ConcretePtr<base_type>>, true));
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<derived_type>, ConcretePtr<base_type>>,
                           ConcretePtr<base_type>
                       >,
                       true)
                );

                expect(eq(std::common_reference_with<ConcretePtr<const derived_type>, ConcretePtr<base_type>>, true));
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<const derived_type>, ConcretePtr<base_type>>,
                           ConcretePtr<const base_type>
                       >,
                       true)
                );

                expect(eq(std::common_reference_with<ConcretePtr<derived_type>, ConcretePtr<const base_type>>, true));
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<derived_type>, ConcretePtr<const base_type>>,
                           ConcretePtr<const base_type>
                       >,
                       true)
                );

                expect(eq(std::common_reference_with<ConcretePtr<const derived_type>, ConcretePtr<volatile base_type>>, true));
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<const derived_type>, ConcretePtr<volatile base_type>>,
                           ConcretePtr<const volatile base_type>
                       >,
                       true)
                );

                expect(eq(std::common_reference_with<ConcretePtr<derived_type>, base_type*>, true));
                expect(eq(std::same_as<std::common_reference_t<ConcretePtr<derived_type>, base_type*>, base_type*>, true));

                expect(eq(std::common_reference_with<derived_type*, ConcretePtr<base_type>>, true));
                expect(eq(std::same_as<std::common_reference_t<derived_type*, ConcretePtr<base_type>>, base_type*>, true));

                expect(eq(std::common_reference_with<ConcretePtr<const derived_type>, volatile base_type*>, true));
                expect(
                    eq(std::same_as<
                           std::common_reference_t<ConcretePtr<const derived_type>, volatile base_type*>,
                           const volatile base_type*
                       >,
                       true)
                );
            });
        };

        //============================================================
        // Arithmetic operations
        //============================================================

        "pointer arithmetic operations according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using t = ConcretePtr<std::int32_t>;

                expect(eq(HasAddition<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasSubtraction<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasDifference<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasPreIncrement<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasPostIncrement<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasPreDecrement<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasPostDecrement<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
            });
        };

        "ordering comparisons according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                expect(
                    eq(std::three_way_comparable<ConcretePtr<std::int32_t>>,
                       pointer_test_traits<ConcretePtr>::has_arithmetic_traversal)
                );
                expect(
                    eq(std::three_way_comparable_with<ConcretePtr<std::int32_t>, std::int32_t*>,
                       pointer_test_traits<ConcretePtr>::has_arithmetic_traversal)
                );
                expect(
                    eq(std::three_way_comparable<ConcretePtr<base_type>>,
                       pointer_test_traits<ConcretePtr>::has_arithmetic_traversal)
                );
                expect(
                    eq(std::three_way_comparable_with<ConcretePtr<base_type>, ConcretePtr<derived_type>>,
                       pointer_test_traits<ConcretePtr>::has_arithmetic_traversal)
                );
                expect(
                    eq(std::three_way_comparable_with<ConcretePtr<base_type>, derived_type*>,
                       pointer_test_traits<ConcretePtr>::has_arithmetic_traversal)
                );
                expect(
                    eq(std::three_way_comparable_with<base_type*, ConcretePtr<derived_type>>,
                       pointer_test_traits<ConcretePtr>::has_arithmetic_traversal)
                );
            });
        };

        "input_or _output_iterator according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                //note: all other iterator concepts subsume this one and thus are implicitly false when it is false
                expect(
                    eq(std::input_or_output_iterator<ConcretePtr<std::int32_t>>,
                       pointer_test_traits<ConcretePtr>::has_arithmetic_traversal)
                );
            });
        };

        //============================================================
        // Traversal identity
        //============================================================

        //NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay): Testing pointer arithmetic and indexing operations.
        "pointer arithmetic preserves native traversal semantics"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
                    //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
                    constexpr std::int32_t values[] = {10, 20, 30, 40};

                    constexpr std::ptrdiff_t step = 2;

                    const auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

                    const auto advanced = ptr + step;
                    auto clone          = ptr;

                    expect(eq(clone, ptr));
                    expect(neq(clone, advanced));

                    clone += step;

                    expect(neq(clone, ptr));
                    expect(eq(clone, advanced));

                    expect(eq(*advanced, values[step]));
                    expect(eq(advanced.get(), values + step));
                    expect(eq(ptr < advanced, true));
                    expect(eq(advanced > ptr, true));
                    expect(eq(advanced >= (values + (step / 2)), true));
                }
            });
        };

        "difference matches raw pointer semantics"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
                    //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
                    std::int32_t values[] = {10, 20, 30, 40};

                    constexpr std::ptrdiff_t first_index = 0;
                    constexpr std::ptrdiff_t last_index  = 3;

                    const auto first = base::vocab::pointer_to<ConcretePtr>(values[first_index]);
                    const auto last  = base::vocab::pointer_to<ConcretePtr>(values[last_index]);

                    expect(eq(last - first, last_index - first_index));
                }
            });
        };

        //NOLINTBEGIN(bugprone-argument-comment): Matchers lhs/rhs.
        "increment and decrement traverse correctly"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
                    //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
                    std::int32_t values[] = {1, 2, 3};

                    auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

                    ++ptr;
                    expect(eq(*ptr, 2));

                    ptr++;
                    expect(eq(*ptr, 3));

                    --ptr;
                    expect(eq(*ptr, 2));

                    ptr--;
                    expect(eq(*ptr, 1));
                }
            });
        };
        //NOLINTEND(bugprone-argument-comment)

        "subscript matches raw pointer indexing"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
                    //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
                    std::int32_t values[] = {5, 6, 7, 8};

                    const auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                    expect(eq(ptr[0], values[0]));
                    expect(eq(ptr[1], values[1]));
                    expect(eq(ptr[2], values[2]));
#pragma GCC diagnostic pop
                }
            });
        };

        "mixed raw and cursor arithmetic produce identical addresses"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
                    //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
                    std::int32_t values[] = {1, 2, 3, 4};

                    const auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

                    expect(eq((ptr + 3).get(), values + 3));
                    expect(eq((3 + ptr).get(), values + 3));
                }
            });
        };
        //NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

        //============================================================
        // CV-correctness propagation
        //============================================================

        "const element forbids mutation through dereference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const std::int32_t value{};
                const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                // Compile-time: *ptr must NOT be assignable
                constexpr bool can_assign = std::is_assignable_v<decltype(*ptr), std::int32_t>;

                expect(eq(can_assign, false));
            });
        };

        "const pointer prevents rebinding but not mutation"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                constexpr auto initial{5};
                constexpr auto expected{10};

                auto value     = initial;
                const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                *ptr = expected;

                constexpr bool can_rebind = std::is_assignable_v<decltype(ptr)&, const decltype(ptr)&>;

                expect(eq(can_rebind, false));

                expect(neq(value, initial));
                expect(eq(value, expected));
            });
        };

        "`address_type` nested type preserves top-level const"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using t = ConcretePtr<const std::int32_t>;

                expect(eq(std::same_as<typename t::address_type, const std::int32_t*>, true));
            });
        };

        "`reference` nested type preserves const"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using t = ConcretePtr<const std::int32_t>;

                expect(eq(std::same_as<typename t::reference, const std::int32_t&>, true));
            });
        };

        //NOLINTBEGIN(misc-const-correctness): Readability suffers with const correctness in this test.
        "qualification climbing construction and assignment"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                std::int32_t value{};
                std::int32_t other{};
                auto mutable_ptr = base::vocab::pointer_to<ConcretePtr, std::int32_t>(value);
                auto const_ptr   = base::vocab::pointer_to<ConcretePtr, const std::int32_t>(other);

                //Qualification climbing (Assignment)
                const_ptr = mutable_ptr;
                expect(eq(const_ptr.get() == mutable_ptr.get(), true));

                //Qualification climbing (Construction)
                ConcretePtr<const std::int32_t> const_copy{mutable_ptr};
                expect(eq(const_copy.get() == mutable_ptr.get(), true));
            });
        };
        //NOLINTEND(misc-const-correctness)

        "volatile qualifier preservation"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                //NOLINTNEXTLINE(readability-magic-numbers): Test fixture needs a meaningless number.
                volatile std::int32_t hardware_register = 0xAA;
                const auto ptr                          = base::vocab::pointer_to<ConcretePtr>(hardware_register);

                //Ensure the raw pointer retrieved is also volatile
                expect(eq(std::same_as<decltype(ptr.get()), volatile std::int32_t*>, true));

                //Ensure conversion to raw pointer preserves volatile
                volatile std::int32_t* raw = ptr;
                expect(eq(raw, std::addressof(hardware_register)));

                //Ensure dereference preserves volatile
                //NOLINTNEXTLINE(misc-const-correctness): It would be missing the point.
                decltype(auto) dereferenced = *ptr;
                expect(eq(std::is_volatile_v<std::remove_reference_t<decltype(dereferenced)>>, true));
                expect(eq(dereferenced, hardware_register));
            });
        };

        "common_type preserves const qualification"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using common_t = std::common_type_t<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>;

                expect(eq(std::same_as<common_t, ConcretePtr<const std::int32_t>>, true));
            });
        };

        "common_reference preserves const qualification"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                using common_ref = std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>;

                expect(eq(std::same_as<common_ref, ConcretePtr<const std::int32_t>>, true));
            });
        };

        //============================================================
        // Interoperability with raw pointer APIs
        //============================================================

        "implicit conversion works with raw pointer API"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const auto takes_ptr = [](const std::int32_t* iptr) { return *iptr; };

                //NOLINTNEXTLINE(readability-magic-numbers): Test fixture needs a meaningless number.
                std::int32_t value = 3;
                const auto ptr     = base::vocab::pointer_to<ConcretePtr>(value);

                expect(eq(takes_ptr(ptr), value));
            });
        };

        "get() works with raw pointer API"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const auto takes_ptr = [](const std::int32_t* iptr) { return *iptr; };

                //NOLINTNEXTLINE(readability-magic-numbers): Test fixture needs a meaningless number.
                std::int32_t value = 4;
                const auto ptr     = base::vocab::pointer_to<ConcretePtr>(value);

                expect(eq(takes_ptr(ptr.get()), value));
            });
        };

        //NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay): Testing interactions with C Arrays, including pointer arithmetic and indexing operations.
        "not constructible, convertible, nor assignable from C-array decay"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                //NOLINTNEXTLINE(readability-magic-numbers): Test fixture.
                std::int32_t array[3] = {0, 1, 2};

                //ConcretePtr<std::int32_t> should_fail{array};

                expect(eq(std::convertible_to<decltype(array), ConcretePtr<std::int32_t>>, false));
                expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, decltype(array)>, false));
                expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t>&, decltype(array)>, false));

                expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, decltype(array[0])>, true));

                const auto ptr = base::vocab::pointer_to<ConcretePtr>(array[1]);

                //Ensure binding to the element is equivalent to expected array-to-pointer decay with pointer offset arithmetic
                expect(eq(ptr.get(), array + 1));
            });
        };

        "not constructible, convertible, nor assignable from C-array decay when pointing to an array"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                //NOLINTBEGIN(readability-magic-numbers): Test fixture.
                std::int32_t array[3][3] = {
                    {0, 1, 2},
                    {3, 4, 5},
                    {6, 7, 8},
                };
                //NOLINTEND(readability-magic-numbers)

                //ConcretePtr<std::int32_t[3]> should_fail{array};

                expect(eq(std::convertible_to<decltype(array), ConcretePtr<std::int32_t[3]>>, false));
                expect(eq(std::constructible_from<ConcretePtr<std::int32_t[3]>, decltype(array)>, false));
                expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t[3]>&, decltype(array)>, false));

                expect(eq(std::constructible_from<ConcretePtr<std::int32_t[3]>, decltype(array[0])>, true));

                const auto ptr = base::vocab::pointer_to<ConcretePtr, std::int32_t[3]>(array[1]);

                //Ensure binding to the element is equivalent to expected array-to-pointer decay with pointer offset arithmetic
                expect(eq(ptr.get(), array + 1));
            });
        };
        //NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

        //============================================================
        // Incomplete types
        //============================================================

        struct incomplete_type;

        "incomplete type support"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    expect(eq(base::meta::concepts::instantiable_with<ConcretePtr, incomplete_type>, true));

                    //NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, readability-magic-numbers): Test requires a fabricated pointer value to an incomplete type.
                    auto* const raw = reinterpret_cast<incomplete_type*>(0x1234);

                    const ConcretePtr<incomplete_type> ptr{raw};

                    expect(eq(ptr.get(), raw));
                }
            });
        };

        struct incomplete_type {
            std::int32_t value;
        };

        "incomplete type becomes usable after completion"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                //NOLINTNEXTLINE(readability-magic-numbers): Test fixture needs a meaningless number.
                incomplete_type obj{42};

                const auto ptr = base::vocab::pointer_to<ConcretePtr>(obj);

                expect(eq(ptr->value, obj.value));
                expect(eq((*ptr).value, obj.value));
            });
        };

        //============================================================
        // `void` support
        //============================================================

        "type aliases are correct for `void`"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    using t = ConcretePtr<void>;

                    constexpr bool element = std::same_as<typename t::element_type, void>;
                    constexpr bool value   = std::same_as<typename t::value_type, void>;
                    constexpr bool pointer = std::same_as<typename t::address_type, void*>;
                    constexpr bool ptrdiff = std::same_as<typename t::difference_type, std::ptrdiff_t>;

                    expect(eq(element, true));
                    expect(eq(value, true));
                    expect(eq(pointer, true));
                    expect(eq(ptrdiff, true));
                }
            });
        };

        "void specialization supports type erasure"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    std::int32_t x{};

                    const auto typed = base::vocab::pointer_to<ConcretePtr>(x);
                    const ConcretePtr<void> erased{typed};

                    expect(eq(erased.get(), static_cast<void*>(std::addressof(x))));
                }
            });
        };

        "void specialization disables dereference operators"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    expect(eq(Dereferenceable<ConcretePtr<base_type>>, true));
                    expect(eq(ArrowAccessible<ConcretePtr<base_type>>, true));

                    expect(eq(Dereferenceable<ConcretePtr<void>>, false));
                    expect(eq(ArrowAccessible<ConcretePtr<void>>, false));
                }
            });
        };

        "construction from void raw pointer is explicit"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    expect(eq(std::convertible_to<void*, ConcretePtr<void>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<void>, void*>, true));
                }
            });
        };

        "construction from void smart pointer is explicit"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    expect(eq(std::convertible_to<trivial_smart_ptr<void>&, ConcretePtr<void>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<void>, trivial_smart_ptr<void>&>, true));
                }
            });
        };

        "void pointer constructs implicitly from typed pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    expect(eq(std::convertible_to<ConcretePtr<std::int32_t>, ConcretePtr<void>>, true));
                    expect(eq(std::convertible_to<ConcretePtr<void>, ConcretePtr<std::int32_t>>, false));

                    const std::int32_t value{42};
                    const auto typed_ptr = base::vocab::pointer_to<ConcretePtr>(value);

                    // Should be implicit (convertible)
                    const auto takes_void = [](ConcretePtr<const void> ptr) { return ptr.get(); };
                    expect(eq(takes_void(typed_ptr), static_cast<const void*>(std::addressof(value))));
                }
            });
        };

        //NOLINTBEGIN(misc-const-correctness): Readability suffers with const correctness in this test.
        "static_pointer_cast converts static pointee type to and from void"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    std::int32_t object = 0;

                    auto source  = base::vocab::pointer_to<ConcretePtr>(object);
                    auto result1 = static_pointer_cast<void>(source);

                    expect(eq(std::same_as<decltype(result1), ConcretePtr<void>>, true));
                    expect(eq(result1.get(), static_cast<void*>(std::addressof(object))));

                    auto result2 = static_pointer_cast<std::int32_t>(result1);

                    expect(eq(std::same_as<decltype(result2), ConcretePtr<std::int32_t>>, true));
                    expect(eq(result2.get(), std::addressof(object)));
                }
            });
        };
        //NOLINTEND(misc-const-correctness)

        "void pointer is equality comparable"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (
                    pointer_test_traits<ConcretePtr>::permits_void_pointee
                    && pointer_test_traits<ConcretePtr>::allows_pointer_binding
                ) {
                    const std::int32_t value{42};
                    const auto typed_ptr                = base::vocab::pointer_to<ConcretePtr>(value);
                    const std::int32_t* const typed_raw = std::addressof(value);
                    const void* const erased_raw        = std::addressof(value);

                    const ConcretePtr<const void> erased_ptr1{typed_raw};
                    const ConcretePtr<const void> erased_ptr2{typed_ptr};

                    expect(eq(erased_ptr1 == erased_ptr2, true));
                    expect(eq(erased_ptr1 == typed_ptr, true));
                    expect(eq(erased_ptr1 == erased_raw, true));
                    expect(eq(erased_ptr1 == typed_raw, true));
                }
            });
        };

        //============================================================
        // CTAD Guide
        //============================================================

        "deduction guides work"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                constexpr std::int32_t value{};

                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    const ConcretePtr ptr1{value};
                    expect(eq(ptr1.get(), std::addressof(value)));
                }

                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    const ConcretePtr ptr2{std::addressof(value)};

                    trivial_smart_ptr<const std::int32_t> smart_pointer{std::addressof(value)};
                    const ConcretePtr ptr3{smart_pointer};

                    expect(eq(ptr2.get(), std::addressof(value)));
                    expect(eq(ptr3.get(), std::addressof(value)));
                }
            });
        };

        //============================================================
        // Constant Expression Usage
        //============================================================

        "constexpr construction and dereference"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                static constexpr std::int32_t value = 42;

                constexpr auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                expect(eq(*ptr, value));
            });
        };

        //NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay): Testing pointer arithmetic and indexing operations.
        "constexpr arithmetic"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
                    //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
                    static constexpr std::int32_t values[] = {2, 4, 6};

                    constexpr auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

                    constexpr auto next = ptr + 1;
                    expect(eq(*next, values[1]));
                }
            });
        };
        //NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

        "constexpr get and boolean conversion"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                static constexpr std::int32_t value = 7;

                constexpr auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                expect(eq(ptr.get(), std::addressof(value)));
                expect(eq(static_cast<bool>(ptr), true));
            });
        };

        "constexpr equality"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                static constexpr std::int32_t value = 11;

                constexpr auto ptr1 = base::vocab::pointer_to<ConcretePtr>(value);
                constexpr auto ptr2 = base::vocab::pointer_to<ConcretePtr>(value);

                expect(eq(ptr1 == ptr2, true));
            });
        };

        "constexpr rebinding"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                static constexpr std::int32_t value1 = 1;
                static constexpr std::int32_t value2 = 2;

                constexpr auto rebound = std::invoke([] {
                    auto ptr = base::vocab::pointer_to<ConcretePtr>(value1);
                    ptr      = ConcretePtr{value2};
                    return ptr;
                });

                expect(eq(*rebound, value2));
                expect(eq(rebound.get(), std::addressof(value2)));
            });
        };

        "constexpr swap"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                static constexpr std::int32_t value1 = 1;
                static constexpr std::int32_t value2 = 2;

                constexpr auto swapped = std::invoke([] {
                    auto lhs = base::vocab::pointer_to<ConcretePtr>(value1);
                    auto rhs = base::vocab::pointer_to<ConcretePtr>(value2);

                    using std::swap;
                    swap(lhs, rhs);

                    return std::pair{lhs, rhs};
                });

                expect(eq(*swapped.first, value2));
                expect(eq(*swapped.second, value1));
            });
        };

        //============================================================
        // Hash Support
        //============================================================

        "hash matches raw pointer hash"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                std::int32_t value{};

                const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                const auto ptr_hash = std::hash<ConcretePtr<std::int32_t>>{}(ptr);
                const auto raw_hash = std::hash<std::int32_t*>{}(std::addressof(value));

                expect(eq(ptr_hash, raw_hash));
            });
        };

        "equal pointers produce equal hashes"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                std::int32_t value{};

                const auto lhs = base::vocab::pointer_to<ConcretePtr, std::int32_t>(value);
                const auto rhs = base::vocab::pointer_to<ConcretePtr, const std::int32_t>(value);

                const auto lhs_hash = std::hash<ConcretePtr<std::int32_t>>{}(lhs);
                const auto rhs_hash = std::hash<ConcretePtr<const std::int32_t>>{}(rhs);

                expect(eq(lhs == rhs, true));
                expect(eq(lhs_hash == rhs_hash, true));
            });
        };

        //============================================================
        // Formatting and output stream support
        //============================================================

        "std::formatter formats as raw pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                std::int32_t value{};

                const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                const auto formatted_ptr = std::format("{}", ptr);
                const auto formatted_raw = std::format<void*>("{}", std::addressof(value));

                expect(eq(formatted_ptr, formatted_raw));
            });
        };

        "std::formatter formats null equivalently to raw pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    ConcretePtr<std::int32_t> ptr{nullptr};

                    const auto formatted_ptr = std::format("{}", ptr);
                    const auto formatted_raw = std::format<void*>("{}", nullptr);

                    expect(eq(formatted_ptr, formatted_raw));
                }
            });
        };

        "std::formatter supports cv-qualified element types"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const std::int32_t value{};

                const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                const auto formatted_ptr = std::format("{}", ptr);
                const auto formatted_raw = std::format<const void*>("{}", std::addressof(value));

                expect(eq(formatted_ptr, formatted_raw));
            });
        };

        "ostream insertion outputs raw pointer representation"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                std::int32_t value{};

                const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                std::ostringstream ptr_stream;
                std::ostringstream raw_stream;

                ptr_stream << ptr;
                raw_stream << std::addressof(value);

                expect(eq(ptr_stream.str(), raw_stream.str()));
            });
        };

        "ostream insertion outputs null equivalently to raw pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    const ConcretePtr<std::int32_t> ptr{nullptr};

                    std::ostringstream ptr_stream;
                    std::ostringstream raw_stream;

                    ptr_stream << ptr;
                    raw_stream << static_cast<std::int32_t*>(nullptr);

                    expect(eq(ptr_stream.str(), raw_stream.str()));
                }
            });
        };

        //NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast): Raw pointer stream inserters lack support for `volatile` pointees, so a `const_cast` to remove the qualifier is required to stream the address value.
        "ostream insertion supports cv-qualified element types"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                volatile std::int32_t value{};

                const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

                std::ostringstream ptr_stream;
                std::ostringstream raw_stream;

                ptr_stream << ptr;
                raw_stream << const_cast<
                    std::add_pointer_t<std::remove_volatile_t<std::remove_pointer_t<decltype(std::addressof(value))>>>
                >(std::addressof(value));

                expect(eq(ptr_stream.str(), raw_stream.str()));
            });
        };
        //NOLINTEND(cppcoreguidelines-pro-type-const-cast)
    };
} //namespace

int main() {}
