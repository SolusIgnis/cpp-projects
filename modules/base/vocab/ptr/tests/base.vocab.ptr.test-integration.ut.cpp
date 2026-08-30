#include <algorithm>

// SPDX-License-Identifier: Apache-2.0
// Integration tests for base.vocab.ptr

import base.vocab.ptr;
import ut;
import std;

import base.meta.concepts;

using namespace ut;
using namespace base::vocab::ptr;

namespace {
    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct mixin_1 {
        virtual ~mixin_1()         = default; //NOLINT(cppcoreguidelines-special-member-functions): Trivial fixture.
        virtual std::int32_t bar() = 0;
    };

    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct mixin_2 {
        virtual ~mixin_2() = default; //NOLINT(cppcoreguidelines-special-member-functions): Trivial fixture.

        [[nodiscard]] std::size_t foo(this auto&& self) { return sizeof(self); }
    };

    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct base_type : mixin_1,
                       mixin_2 {
        ~base_type() override = default;

        static inline constexpr std::int32_t default_value{0};
        std::int32_t value{default_value};

        [[nodiscard]] std::int32_t bar() override { return value; }
    };

    struct derived_type : base_type {
        static inline constexpr std::int32_t default_extra{42};
        std::int32_t extra{default_extra};
        std::array<std::byte, sizeof(base_type) + alignof(base_type)> make_derived_bigger_than_base_even_with_tail_padding{};

        [[nodiscard]] std::int32_t bar() override { return extra; }

        [[nodiscard]] std::size_t baz() const { return make_derived_bigger_than_base_even_with_tail_padding.size(); }
    };

    template<typename T, std::size_t N>
    class static_buffer {
        static_assert(N > 0, "Why would you want an empty `static_buffer`?");

    public:
        using size_type      = std::size_t;
        using pointer        = cursor_ptr<T>;
        using const_pointer  = cursor_ptr<const T>;
        using iterator       = iterator_ptr<T>;
        using const_iterator = iterator_ptr<const T>;
        using element_type   = pointer::element_type;
        using value_type     = pointer::value_type;

    private:
        //NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays): This fixture needs raw storage to test the vocabulary pointers. It is essentially implementing an analogue to `std::array`.
        element_type storage[N]{};

    public:
        [[nodiscard]] constexpr size_type size() const noexcept { return N; }

        [[nodiscard]] constexpr bool empty() const noexcept { return N == 0; }

        [[nodiscard]] constexpr iterator begin() noexcept { return iterator_ptr{std::addressof(storage[0])}; }

        [[nodiscard]] constexpr iterator end() noexcept { return begin() + N; }

        [[nodiscard]] constexpr const_iterator begin() const noexcept { return iterator_ptr{std::addressof(storage[0])}; }

        [[nodiscard]] constexpr const_iterator end() const noexcept { return begin() + N; }

        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }

        [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

        constexpr pointer data() noexcept { return cursor_ptr{storage[0]}; }

        [[nodiscard]] constexpr const_pointer data() const noexcept { return cursor_ptr{storage[0]}; }

        [[nodiscard]] constexpr const_pointer cdata() const noexcept { return data(); }
    };

    //NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
    suite vocabulary_pointer_integration_tests = [] mutable {
        "vocabulary pointers interoperate to model object relationships"_test = [] mutable {
            struct dummy_type {
                dependency_ptr<base_type> service;
                alias_ptr<std::size_t> counter = {};
            };

            base_type base_service;

            dummy_type dummy_obj{.service = dependency_ptr{base_service}};

            constexpr std::int32_t expected_bar_val = 11;
            derived_type derived_service;
            derived_service.extra = expected_bar_val;
            std::size_t count     = 0;
            dummy_obj.service     = dependency_ptr{derived_service};

            auto dummy_ptr = base::vocab::pointer_to<required_ptr>(dummy_obj);
            expect(eq(dummy_ptr->counter == nullptr, true));
            dummy_ptr->counter = std::addressof(count);

            expect(eq(*dummy_ptr->counter, /*rhs=*/0ZU));

            required_ptr local_counter = dummy_ptr->counter;
            (*local_counter)++;

            expect(eq(*dummy_ptr->counter, /*rhs=*/1ZU));

            (*dummy_obj.counter)++;

            expect(eq(*dummy_ptr->counter, /*rhs=*/2ZU));
            expect(eq(count, /*rhs=*/2ZU));

            static_assert(
                sizeof(derived_type) > sizeof(base_type),
                "`derived_type` must be larger than `base_type` for this test to be valid."
            );
            expect(eq(dummy_ptr->service->foo(), sizeof(base_type)));
            expect(eq(dummy_ptr->service->bar(), expected_bar_val));
        };

        //NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access): Testing subscripting and pointer arithmetic integration with the vocabulary pointers.
        "vocabulary pointers compose through conversion, container storage, and iteration"_test = [] mutable {
            base_type bt1;
            base_type bt2;
            derived_type dt1;
            derived_type dt2;

            std::vector<required_ptr<base_type>> vec;

            dependency_ptr<base_type> dpbt1{bt1};

            vec.emplace_back(dpbt1);
            vec.emplace_back(required_ptr{dt1});
            vec.emplace_back(std::addressof(bt2));
            vec.emplace_back(std::addressof(dt2));

            // converts argument from cursor_ptr<required_ptr<base_type>> to required_ptr<required_ptr<base_type>>
            const auto unwrap = [](required_ptr<required_ptr<base_type>> param) { return *param; };

            for (auto [cursor, i] = std::tuple{cursor_ptr{vec.data()}, 0ZU}; i < vec.size(); ++cursor, ++i) {
                const bool is_derived = (i == 1) || (i == 3); // 0 and 2 are base_type; 1 and 3 are derived_type
                expect(eq(unwrap(cursor)->value, base_type::default_value));
                expect(eq(unwrap(cursor)->bar(), (is_derived ? derived_type::default_extra : base_type::default_value)));
            }

            constexpr std::int32_t new_value{1};
            alias_ptr alias = vec[1];
            alias->value    = new_value;
            expect(eq(dt1.value, new_value));
            expect(eq(alias->bar(), derived_type::default_extra));
        };
        //NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

        //NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access): Testing subscripting and pointer arithmetic integration with the vocabulary pointers.
        "vocabulary pointers interoperate with unordered associative containers"_test = [] mutable {
            std::unordered_set<alias_ptr<const char>> test_set;
            //Note, we get an automatic null check when converting to `required_ptr` to access the map.
            std::unordered_map<required_ptr<const char>, std::int64_t> test_map;

            constexpr auto data = "This is a test.";
            std::string_view sview{data};

            const auto i_count = std::ranges::count(sview, 'i');
            const auto s_count = std::ranges::count(sview, 's');
            const auto t_count = std::ranges::count(sview, 't');

            for (cursor_ptr<const char> datum{data[0]}; *datum != '\0'; ++datum) {
                //Convert the `cursor_ptr` to `required_ptr` to populate the map.
                test_map.emplace(datum, std::ranges::count(sview, *datum));
                //Convert the `cursor_ptr` to `alias_ptr` to populate the set.
                test_set.emplace(datum);
            }

            alias_ptr<const char> lookup;

            {
                bool threw_when_null = false;
                bool wrong_exception = false;
                try {
                    [[maybe_unused]] std::int32_t unused = test_map[lookup];
                } catch (const std::invalid_argument&) {
                    threw_when_null = true;
                } catch (...) {
                    wrong_exception = true;
                }

                expect(eq(threw_when_null, true));
                expect(eq(wrong_exception, false));
            }

            lookup = (data + 5); //NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-avoid-magic-numbers)

            expect(eq(test_map[lookup], i_count));
            expect(eq(test_map[required_ptr{data[6]}], s_count)); //NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-avoid-magic-numbers)
            expect(eq(test_map[cursor_ptr{data} + 10], t_count)); //NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-avoid-magic-numbers)

            expect(eq(test_set.contains(lookup), true));
            expect(eq(test_set.size(), sview.size()));

            test_set.emplace(lookup);

            expect(eq(test_set.size(), sview.size()));

            test_set.emplace(nullptr);

            expect(eq(test_set.contains(nullptr), true));
            expect(eq(test_set.size(), sview.size() + 1));
        };
        //NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

        "`common_reference_t` with mixed vocabulary pointer types resolves correctly"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(std::common_reference_with<dependency_ptr<simple_t>, required_ptr<simple_t>>, true));
            expect(eq(std::common_reference_with<dependency_ptr<simple_t>, alias_ptr<simple_t>>, true));
            expect(eq(std::common_reference_with<dependency_ptr<simple_t>, cursor_ptr<simple_t>>, true));
            expect(eq(std::common_reference_with<dependency_ptr<simple_t>, iterator_ptr<simple_t>>, true));
            expect(eq(std::common_reference_with<required_ptr<simple_t>, alias_ptr<simple_t>>, true));
            expect(eq(std::common_reference_with<required_ptr<simple_t>, cursor_ptr<simple_t>>, true));
            expect(eq(std::common_reference_with<required_ptr<simple_t>, iterator_ptr<simple_t>>, true));
            expect(eq(std::common_reference_with<alias_ptr<simple_t>, cursor_ptr<simple_t>>, true));
            expect(eq(std::common_reference_with<alias_ptr<simple_t>, iterator_ptr<simple_t>>, true));
            expect(eq(std::common_reference_with<cursor_ptr<simple_t>, iterator_ptr<simple_t>>, true));

            expect(
                eq(std::same_as<
                       std::common_reference_t<dependency_ptr<simple_t>, dependency_ptr<simple_t>>,
                       dependency_ptr<simple_t>
                   >,
                   true)
            );
            expect(
                eq(std::same_as<
                       std::common_reference_t<dependency_ptr<simple_t>, required_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(
                eq(std::same_as<
                       std::common_reference_t<dependency_ptr<simple_t>, alias_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(
                eq(std::same_as<
                       std::common_reference_t<dependency_ptr<simple_t>, cursor_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(
                eq(std::same_as<
                       std::common_reference_t<dependency_ptr<simple_t>, iterator_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );

            expect(
                eq(std::same_as<
                       std::common_reference_t<required_ptr<simple_t>, dependency_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(eq(
                std::same_as<std::common_reference_t<required_ptr<simple_t>, required_ptr<simple_t>>, required_ptr<simple_t>>,
                true
            ));
            expect(eq(
                std::
                    same_as<std::common_reference_t<required_ptr<simple_t>, alias_ptr<simple_t>>, std::add_pointer_t<simple_t>>,
                true
            ));
            expect(
                eq(std::same_as<
                       std::common_reference_t<required_ptr<simple_t>, cursor_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(
                eq(std::same_as<
                       std::common_reference_t<required_ptr<simple_t>, iterator_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );

            expect(
                eq(std::same_as<
                       std::common_reference_t<alias_ptr<simple_t>, dependency_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(eq(
                std::
                    same_as<std::common_reference_t<alias_ptr<simple_t>, required_ptr<simple_t>>, std::add_pointer_t<simple_t>>,
                true
            ));
            expect(
                eq(std::same_as<std::common_reference_t<alias_ptr<simple_t>, alias_ptr<simple_t>>, alias_ptr<simple_t>>, true)
            );
            expect(eq(
                std::same_as<std::common_reference_t<alias_ptr<simple_t>, cursor_ptr<simple_t>>, std::add_pointer_t<simple_t>>,
                true
            ));
            expect(eq(
                std::
                    same_as<std::common_reference_t<alias_ptr<simple_t>, iterator_ptr<simple_t>>, std::add_pointer_t<simple_t>>,
                true
            ));

            expect(
                eq(std::same_as<
                       std::common_reference_t<cursor_ptr<simple_t>, dependency_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(
                eq(std::same_as<
                       std::common_reference_t<cursor_ptr<simple_t>, required_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(eq(
                std::same_as<std::common_reference_t<cursor_ptr<simple_t>, alias_ptr<simple_t>>, std::add_pointer_t<simple_t>>,
                true
            ));
            expect(
                eq(std::same_as<std::common_reference_t<cursor_ptr<simple_t>, cursor_ptr<simple_t>>, cursor_ptr<simple_t>>,
                   true)
            );
            expect(
                eq(std::same_as<
                       std::common_reference_t<cursor_ptr<simple_t>, iterator_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );

            expect(
                eq(std::same_as<
                       std::common_reference_t<iterator_ptr<simple_t>, dependency_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(
                eq(std::same_as<
                       std::common_reference_t<iterator_ptr<simple_t>, required_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(eq(
                std::
                    same_as<std::common_reference_t<iterator_ptr<simple_t>, alias_ptr<simple_t>>, std::add_pointer_t<simple_t>>,
                true
            ));
            expect(
                eq(std::same_as<
                       std::common_reference_t<iterator_ptr<simple_t>, cursor_ptr<simple_t>>,
                       std::add_pointer_t<simple_t>
                   >,
                   true)
            );
            expect(eq(
                std::same_as<std::common_reference_t<iterator_ptr<simple_t>, iterator_ptr<simple_t>>, iterator_ptr<simple_t>>,
                true
            ));

            expect(
                eq(std::same_as<
                       std::common_reference_t<dependency_ptr<base_type>, required_ptr<const derived_type>>,
                       std::add_pointer_t<const base_type>
                   >,
                   true)
            );
            expect(
                eq(std::same_as<
                       std::common_reference_t<required_ptr<const base_type>, dependency_ptr<volatile derived_type>>,
                       std::add_pointer_t<const volatile base_type>
                   >,
                   true)
            );
            expect(
                eq(std::same_as<
                       std::common_reference_t<alias_ptr<derived_type>, required_ptr<volatile base_type>>,
                       std::add_pointer_t<volatile base_type>
                   >,
                   true)
            );
        };

        "vocabulary pointers alias external owning pointers"_test = [] mutable {
            auto unique = std::make_unique<derived_type>();
            auto shared = std::make_shared<derived_type>();

            required_ptr<base_type> required = unique;
            alias_ptr<base_type> alias       = shared;

            expect(eq(required->bar(), unique->bar()));
            expect(eq(alias->bar(), shared->bar()));

            constexpr std::int32_t u_expected = 17;
            required->value                   = u_expected;
            expect(eq(unique->value, u_expected));

            constexpr std::int32_t s_expected = 29;
            alias->value                      = s_expected;
            expect(eq(shared->value, s_expected));
        };

        "nullability policy is enforced from owning smart pointers"_test = [] mutable {
            std::unique_ptr<std::int32_t> empty_unique{};

            { //always-engaged pointer
                bool threw           = false;
                bool wrong_exception = false;
                try {
                    [[maybe_unused]] const required_ptr dummy_ptr = empty_unique;
                } catch (const std::invalid_argument&) {
                    threw = true;
                } catch (...) {
                    wrong_exception = true;
                }

                expect(eq(threw, true));
                expect(eq(wrong_exception, false));
            } //always-engaged pointer

            { //nullable pointer
                bool threw           = false;
                bool wrong_exception = false;
                try {
                    const alias_ptr ptr = alias_ptr{empty_unique};
                    expect(eq(ptr == nullptr, true));
                } catch (const std::invalid_argument&) {
                    threw = true;
                } catch (...) {
                    wrong_exception = true;
                }

                expect(eq(threw, false));
                expect(eq(wrong_exception, false));
            } //nullable pointer
        };

        "vocabulary pointers observe but never participate in ownership"_test = [] mutable {
            constexpr std::int32_t expected = 21;

            auto shared               = std::make_shared<derived_type>();
            const auto original_count = shared.use_count();

            {
                alias_ptr alias = shared;
                expect(eq(shared.use_count(), original_count));
                alias->extra = expected;
            } //~alias
            expect(eq(shared.use_count(), original_count));
            expect(eq(shared->extra, expected));

            auto owner                   = std::make_unique<derived_type>();
            required_ptr<base_type> ptr1 = owner;

            {
                required_ptr<base_type> ptr2 = owner;
                ptr2->value                  = expected;
            } //~ptr2

            auto owner2 = std::move(owner);

            expect(eq(owner == nullptr, true));
            expect(eq(std::to_address(owner2) == std::to_address(ptr1), true));
            expect(eq(owner2->bar(), ptr1->bar()));
            expect(eq(owner2->value, expected));
        };

        "`iterator_ptr` is a contiguous iterator"_test = [] mutable {
            expect(eq(std::input_or_output_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::input_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::input_iterator<iterator_ptr<const std::int32_t>>, true));
            expect(eq(std::output_iterator<iterator_ptr<std::int32_t>, std::int32_t>, true));
            expect(eq(std::output_iterator<iterator_ptr<const std::int32_t>, std::int32_t>, false));
            expect(eq(std::sized_sentinel_for<iterator_ptr<std::int32_t>, iterator_ptr<std::int32_t>>, true));
            expect(eq(std::forward_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::bidirectional_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::random_access_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::contiguous_iterator<iterator_ptr<std::int32_t>>, true));
        };

        "`cursor_ptr` and `iterator_ptr` interoperate with standard algorithms"_test = [] mutable {
            constexpr std::size_t buffer_size{8};
            constexpr auto find_target{7};
            constexpr auto lb_target{6};
            constexpr std::array<std::int32_t, buffer_size> source{5, 2, 8, 1, 7, 4, 6, 3};

            auto expected_sorted = source;
            std::ranges::sort(expected_sorted);

            auto expected_reversed = source;
            std::ranges::reverse(expected_reversed);

            static_buffer<std::int32_t, buffer_size> buffer;

            // Copy raw-pointer iterators -> `cursor_ptr` into underlying memory sequence
            std::ranges::copy(source, buffer.data());

            // Reverse with `iterator_ptr` iterators in and out
            static_assert(std::sentinel_for<decltype(buffer.end()), decltype(buffer.begin())>);
            static_buffer<std::int32_t, buffer_size> reverse_result;
            std::ranges::reverse_copy(buffer, reverse_result.begin());
            expect(eq(std::ranges::equal(reverse_result, expected_reversed), true));

            // Search using `iterator_ptr` iterators
            auto fpos = std::ranges::find(buffer, find_target);
            expect(eq(fpos != buffer.end(), true));
            if (fpos != buffer.end()) {
                expect(eq(*fpos, find_target));
            }

            // Sort using `iterator_ptr` iterators
            std::ranges::sort(buffer);

            // Binary search using `iterator_ptr` iterators
            auto lbpos = std::ranges::lower_bound(buffer, lb_target);
            expect(eq(lbpos != buffer.end(), true));
            if (lbpos != buffer.end()) {
                expect(eq(*lbpos, lb_target));
            }

            // Copy `iterator_ptr` iterator -> raw-pointer iterator
            std::vector<std::int32_t> sort_result;
            std::ranges::copy(buffer, std::back_inserter(sort_result));

            expect(eq(std::ranges::equal(sort_result, expected_sorted), true));

            // Partition the buffer by evenness: `iterator_ptr` iterators form a valid range
            const auto is_even   = [](int x) { return x % 2 == 0; };
            const auto remainder = std::ranges::partition(buffer, is_even);

            // Expect everything before and nothing after the partition point to be even
            expect(eq(std::ranges::all_of(buffer.begin(), remainder.begin(), is_even), true));
            expect(eq(std::ranges::none_of(remainder, is_even), true));
        };

        "`iterator_ptr` interoperates with `std::span`"_test = [] mutable {
            constexpr std::size_t buffer_size{8};
            constexpr std::array<std::int32_t, buffer_size> source{5, 2, 8, 1, 7, 4, 6, 3};
            static_buffer<std::int32_t, buffer_size> buffer;

            std::ranges::copy(source, buffer.data());

            const std::span view(buffer);

            expect(eq(view.front(), source.front()));
            expect(eq(view.back(), source.back()));
        };

        "`iterator_ptr` iterators interoperate with standard views"_test = [] mutable {
            constexpr std::size_t buffer_size{8};
            constexpr std::array<std::int32_t, buffer_size> source{5, 2, 8, 1, 7, 4, 6, 3};

            const auto is_even  = [](int x) { return x % 2 == 0; };
            const auto expected = source | std::views::filter(is_even) | std::ranges::to<std::vector>();

            static_buffer<std::int32_t, buffer_size> buffer;

            // Fill the buffer.
            std::ranges::copy(source, buffer.begin());

            // Build a filtering view with the buffer's `iterator_ptr` iterators and materialize the range into a vector.
            const auto even_values = buffer | std::views::filter(is_even) | std::ranges::to<std::vector>();

            // The result when filtering with `iterator_ptr` iterators should be the same as when filtering with raw-pointer iterators.
            expect(eq(std::ranges::equal(even_values, expected), true));
        };

        "vocabulary pointer casts and conversions preserve object representation"_test = [] mutable {
            const derived_type object;

            const auto expected = std::as_bytes(std::span{std::addressof(object), 1});

            const auto object_ptr     = base::vocab::pointer_to<required_ptr>(object);
            const cursor_ptr byte_ptr = reinterpret_pointer_cast<std::byte>(object_ptr);

            static_buffer<std::byte, sizeof(derived_type)> buffer;
            std::copy(byte_ptr, byte_ptr + sizeof(derived_type), buffer.data());

            expect(eq(std::ranges::equal(buffer, expected), true));
        };
    };
} //namespace

int main() {}
