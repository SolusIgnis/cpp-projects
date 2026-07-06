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
        int value{0};
    };

    struct derived_type : base_type {
        int extra{42};
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
        "`iterator_ptr` is a contiguous iterator"_test = [] mutable {
            expect(eq(std::input_or_output_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::input_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::input_iterator<iterator_ptr<const std::int32_t>>, true));
            expect(eq(std::output_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::output_iterator<iterator_ptr<const std::int32_t>>, false));
            expect(eq(std::sized_sentinel_for<iterator_ptr<std::int32_t>, iterator_ptr<std::int32_t>>, true));
            expect(eq(std::forward_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::bidirectional_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::random_access_iterator<iterator_ptr<std::int32_t>>, true));
            expect(eq(std::contiguous_iterator<iterator_ptr<std::int32_t>>, true));
        };

        "`cursor_ptr` and `iterator_ptr` iterators interoperate with standard algorithms"_test = [] mutable {
            std::array<std::int32_t, 8> source{5, 2, 8, 1, 7, 4, 6, 3};

            auto expected_sorted = source;
            std::ranges::sort(expected_sorted);

            auto expected_reversed = source;
            std::ranges::reverse(expected_reversed);

            static_buffer<std::int32_t, 8> buffer;

            // Copy raw-pointer iterators -> cursor_ptr
            std::copy(source.begin(), source.end(), buffer.data());

            // Reverse with iterator_ptr iterators in and out
            static_assert(std::sentinel_for<decltype(buffer.end()), decltype(buffer.begin())>);
            static_buffer<std::int32_t, 8> reverse_result;
            std::ranges::reverse_copy(buffer, reverse_result.begin());
            expect(eq(std::ranges::equal(reverse_result, expected_reversed), true));

            // Search using iterator_ptr iterators
            auto fpos = std::ranges::find(buffer, 7);
            expect(eq(fpos != buffer.end(), true));
            if (fpos != buffer.end()) {
                expect(eq(*fpos, 7));
            }

            // Sort using iterator_ptr iterators
            std::ranges::sort(buffer);

            // Binary search using iterator_ptr iterators
            auto lbpos = std::ranges::lower_bound(buffer, 6);
            expect(eq(lbpos != buffer.end(), true));
            if (lbpos != buffer.end()) {
                expect(eq(*lbpos, 6));
            }

            // Copy iterator_ptr -> raw-pointer iterator
            std::vector<std::int32_t> sort_result;
            std::ranges::copy(buffer, std::back_inserter(sort_result));

            expect(eq(std::ranges::equal(sort_result, expected_sorted), true));

            // Partition the buffer by evenness
            auto is_even = [](int x) { return x % 2 == 0; };
            auto remainder = std::ranges::partition(buffer, is_even);

            // Expect everything before and nothing after the partition point to be even
            expect(eq(std::ranges::all_of(buffer.begin(), remainder.begin(), is_even), true));
            expect(eq(std::ranges::none_of(remainder, is_even), true));
        };

        "`iterator_ptr` iterators interoperate with standard views"_test = [] mutable {
            std::array<std::int32_t, 8> source{5, 2, 8, 1, 7, 4, 6, 3};
            static_buffer<std::int32_t, 8> buffer;
            std::vector expected{2, 8, 4, 6};

            std::ranges::copy(source, buffer.begin());

            auto even_values = buffer
                             | std::views::filter([](int x) { return x % 2 == 0; })
                             | std::ranges::to<std::vector>()
                             ;

            expect(eq(std::ranges::equal(even_values, expected), true));
        };
    };
} //namespace

int main() {}
