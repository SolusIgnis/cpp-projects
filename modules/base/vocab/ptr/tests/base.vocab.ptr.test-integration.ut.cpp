// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import ut;
import std;

import base.meta.concepts;

using namespace ut;
using namespace base::vocab::ptr;

namespace {
    struct base_type {
        std::int32_t value{0};
        
        std::size_t foo(this auto&& self) { return sizeof(self); }
        virtual std::int32_t bar() { return value; }
    };

    struct derived_type : base_type {
        std::int32_t extra{42};
        
        std::int32_t bar() override { return extra; }
    };

    template<typename T, std::size_t N>
    class static_buffer {
    public:
        using size_type       = std::size_t;
        using pointer         = cursor_ptr<T>;
        using const_pointer   = cursor_ptr<const T>;
        using iterator        = iterator_ptr<T>;
        using const_iterator  = iterator_ptr<const T>;
        using element_type    = typename pointer::element_type;
        using value_type      = typename pointer::value_type;

    private:
        element_type storage[N]{};

    public:
        constexpr size_type size() const noexcept { return N; }
        constexpr bool empty() const noexcept { return N == 0; }

        constexpr iterator begin() noexcept { return iterator_ptr{storage[0]}; }
        constexpr iterator end() noexcept { return begin() + N; }

        constexpr const_iterator begin() const noexcept { return iterator_ptr{storage[0]}; }
        constexpr const_iterator end() const noexcept { return begin() + N; }
    
        constexpr const_iterator cbegin() const noexcept { return begin(); }
        constexpr const_iterator cend() const noexcept { return end(); }

        constexpr pointer data() noexcept { return cursor_ptr{storage[0]}; }
        constexpr const_pointer data() const noexcept { return cursor_ptr{storage[0]}; }
        constexpr const_pointer cdata() const noexcept { return data(); }
    };
   
    suite vocabulary_pointer_integration_tests = [] mutable {
        "vocabulary pointers interoperate to model object relationships"_test = [] mutable {
            struct dummy_type {
                dependency_ptr<base_type> service;
                alias_ptr<std::size_t> counter = {};
            };

            constexpr std::int32_t expected_bar_val = 11;
            derived_type derived_service;
            derived_service.extra = expected_bar_val;
            std::size_t count = 0;
            dummy_type dummy_obj{ .service = dependency_ptr{derived_service} };
            required_ptr dummy_ptr = std::addressof(dummy_obj);

            expect(eq(dummy_ptr->counter == nullptr, true));
            dummy_ptr->counter = std::addressof(count);

            expect(eq(*dummy_ptr->counter, 0zu));

            required_ptr local_counter = dummy_ptr->counter;
            (*local_counter)++;

            expect(eq(*dummy_ptr->counter, 1zu));
            
            (*dummy_obj.counter)++;

            expect(eq(*dummy_ptr->counter, 2zu));
            expect(eq(count, 2zu));

            expect(eq(dummy_ptr->service->foo(), sizeof(derived_type)));
            expect(eq(dummy_ptr->service->bar(), expected_bar_val));
        };

        "vocabulary pointers compose through conversion, container storage, and iteration"_test = [] mutable {
            base_type bt1;
            base_type bt2;
            derived_type dt1;
            derived_type dt2;

            std::vector<required_ptr<base_type>> vec;

            dependency_ptr<base_type> dpbt1{bt1};

            vec.push_back(dpbt1);
            vec.push_back(required_ptr{dt1});
            vec.push_back(std::addressof(bt2));
            vec.push_back(std::addressof(dt2));

            // converts argument from cursor_ptr<required_ptr<base_type>> to required_ptr<required_ptr<base_type>>
            auto unwrap = [](required_ptr<required_ptr<base_type>> param){ return *param; };

            for(auto [cursor, i] = std::tuple{cursor_ptr{vec.data()}, 0zu}; i < vec.size(); ++cursor, ++i) {
                bool is_derived = (i == 1) || (i == 3); // 0 and 2 are base_type; 1 and 3 are derived_type
                expect(eq(unwrap(cursor)->value, 0));
                expect(eq(unwrap(cursor)->bar(), (is_derived ? 42 : 0)));
            }

            alias_ptr alias = vec[1];
            alias->value = 1;
            expect(eq(dt1.value, 1));
            expect(eq(alias->bar(), 42));
        };

        "basic_common_reference with mixed vocabulary pointer types"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(std::same_as<std::common_reference_t<dependency_ptr<simple_t>, dependency_ptr<simple_t>>, dependency_ptr<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<dependency_ptr<simple_t>, required_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<dependency_ptr<simple_t>, alias_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<dependency_ptr<simple_t>, cursor_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<dependency_ptr<simple_t>, iterator_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));

            expect(eq(std::same_as<std::common_reference_t<required_ptr<simple_t>, dependency_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<required_ptr<simple_t>, required_ptr<simple_t>>, required_ptr<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<required_ptr<simple_t>, alias_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<required_ptr<simple_t>, cursor_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<required_ptr<simple_t>, iterator_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));

            expect(eq(std::same_as<std::common_reference_t<alias_ptr<simple_t>, dependency_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<alias_ptr<simple_t>, required_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<alias_ptr<simple_t>, alias_ptr<simple_t>>, alias_ptr<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<alias_ptr<simple_t>, cursor_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<alias_ptr<simple_t>, iterator_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));

            expect(eq(std::same_as<std::common_reference_t<cursor_ptr<simple_t>, dependency_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<cursor_ptr<simple_t>, required_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<cursor_ptr<simple_t>, alias_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<cursor_ptr<simple_t>, cursor_ptr<simple_t>>, cursor_ptr<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<cursor_ptr<simple_t>, iterator_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));

            expect(eq(std::same_as<std::common_reference_t<iterator_ptr<simple_t>, dependency_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<iterator_ptr<simple_t>, required_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<iterator_ptr<simple_t>, alias_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<iterator_ptr<simple_t>, cursor_ptr<simple_t>>, std::add_pointer_t<simple_t>>, true));
            expect(eq(std::same_as<std::common_reference_t<iterator_ptr<simple_t>, iterator_ptr<simple_t>>, iterator_ptr<simple_t>>, true));

            expect(eq(std::same_as<std::common_reference_t<dependency_ptr<base_type>, required_ptr<const derived_type>>, std::add_pointer_t<const base_type>>, true));
            expect(eq(std::same_as<std::common_reference_t<required_ptr<const  base_type>, dependency_ptr<volatile derived_type>>, std::add_pointer_t<const volatile base_type>>, true));
            expect(eq(std::same_as<std::common_reference_t<alias_ptr<derived_type>, required_ptr<volatile base_type>>, std::add_pointer_t<volatile base_type>>, true));
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
            constexpr std::array<std::int32_t, 8> source{5, 2, 8, 1, 7, 4, 6, 3};

            auto expected_sorted = source;
            std::ranges::sort(expected_sorted);

            auto expected_reversed = source;
            std::ranges::reverse(expected_reversed);

            static_buffer<std::int32_t, 8> buffer;

            // Copy raw-pointer iterators -> `cursor_ptr` into underlying memory sequence
            std::copy(source.begin(), source.end(), buffer.data());

            // Reverse with `iterator_ptr` iterators in and out
            static_assert(std::sentinel_for<decltype(buffer.end()), decltype(buffer.begin())>);
            static_buffer<std::int32_t, 8> reverse_result;
            std::ranges::reverse_copy(buffer, reverse_result.begin());
            expect(eq(std::ranges::equal(reverse_result, expected_reversed), true));

            // Search using `iterator_ptr` iterators
            auto fpos = std::ranges::find(buffer, 7);
            expect(eq(fpos != buffer.end(), true));
            if (fpos != buffer.end()) {
                expect(eq(*fpos, 7));
            }

            // Sort using `iterator_ptr` iterators
            std::ranges::sort(buffer);

            // Binary search using `iterator_ptr` iterators
            auto lbpos = std::ranges::lower_bound(buffer, 6);
            expect(eq(lbpos != buffer.end(), true));
            if (lbpos != buffer.end()) {
                expect(eq(*lbpos, 6));
            }

            // Copy `iterator_ptr` iterator -> raw-pointer iterator
            std::vector<std::int32_t> sort_result;
            std::ranges::copy(buffer, std::back_inserter(sort_result));

            expect(eq(std::ranges::equal(sort_result, expected_sorted), true));

            // Partition the buffer by evenness: `iterator_ptr` iterators form a valid range
            auto is_even = [](int x) { return x % 2 == 0; };
            auto remainder = std::ranges::partition(buffer, is_even);

            // Expect everything before and nothing after the partition point to be even
            expect(eq(std::ranges::all_of(buffer.begin(), remainder.begin(), is_even), true));
            expect(eq(std::ranges::none_of(remainder, is_even), true));
        };

        "`iterator_ptr` interoperates with `std::span`"_test = [] mutable {
            constexpr std::array<std::int32_t, 8> source{5, 2, 8, 1, 7, 4, 6, 3};
            static_buffer<std::int32_t, 8> buffer;

            std::ranges::copy(source, buffer.data());

            std::span view(buffer);

            expect(eq(view.front(), source.front()));
            expect(eq(view.back(), source.back()));
        };

        "`iterator_ptr` iterators interoperate with standard views"_test = [] mutable {
            constexpr std::array<std::int32_t, 8> source{5, 2, 8, 1, 7, 4, 6, 3};

            auto is_even = [](int x) { return x % 2 == 0; };
            auto expected = source
                          | std::views::filter(is_even)
                          | std::ranges::to<std::vector>()
                          ;

            static_buffer<std::int32_t, 8> buffer;
            
            // Fill the buffer.
            std::ranges::copy(source, buffer.begin());

            // Build a filtering view with the buffer's `iterator_ptr` iterators and materialize the range into a vector.
            auto even_values = buffer
                             | std::views::filter(is_even)
                             | std::ranges::to<std::vector>()
                             ;

            // The result when filtering with `iterator_ptr` iterators should be the same as when filtering with raw-pointer iterators.
            expect(eq(std::ranges::equal(even_values, expected), true));
        };
    };
} //namespace

int main() {}
