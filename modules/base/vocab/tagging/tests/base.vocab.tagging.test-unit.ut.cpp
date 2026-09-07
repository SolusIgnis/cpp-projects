// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

import base.vocab.tagging;
import ut;
import std;

using namespace ut;

namespace {
    //Move-only target type used to verify destructive extraction
    struct move_only_t {
        std::int32_t value{0};

        constexpr explicit move_only_t(std::int32_t v) : value(v) {}
        
        constexpr move_only_t(const move_only_t&) = delete;
        constexpr move_only_t& operator=(const move_only_t&) = delete;
    
        constexpr move_only_t(move_only_t&& other) noexcept = default;
    
        constexpr move_only_t& operator=(move_only_t&& other) noexcept = default;
    };

    struct point {
        std::int32_t x;
        std::int32_t y;
        constexpr point(std::int32_t x_val, std::int32_t y_val) : x(x_val), y(y_val) {}
    };
    
    struct position {
    private:
        struct longitude_tag;
        struct elevation_tag;
        struct latitude_tag;
        
        using coordinate_t = std::int64_t;

    public:
        using longitude_t = base::vocab::tagged_boundary<longitude_tag, coordinate_t>;
        using elevation_t = base::vocab::tagged_boundary<elevation_tag, coordinate_t>;
        using latitude_t  = base::vocab::tagged_boundary<latitude_tag, coordinate_t>;

        coordinate_t x{0}; //-west to +east
        coordinate_t y{0}; //-down to +up
        coordinate_t z{0}; //-north to +south

        constexpr position() = default;

        //"Pass-by-value and move" idiom automatically unwraps the tagged int values
        constexpr position(longitude_t x_val, elevation_t y_val, latitude_t z_val) 
            : x(std::move(x_val))
            , y(std::move(y_val))
            , z(std::move(z_val))
        {}
    };

    struct test_tag;
    struct begin_tag;
    struct end_tag;
    
    using first_point = base::vocab::tagged_boundary<begin_tag, point>;
    using last_point  = base::vocab::tagged_boundary<end_tag, point>;
    using first_pos   = base::vocab::tagged_boundary<begin_tag, const position&>;
    using last_pos    = base::vocab::tagged_boundary<end_tag, const position&>;

    constexpr auto distance(first_point begin, last_point end)
    {
        const point first = std::move(begin);
        const point last  = std::move(end);
        
        const auto x_dist = last.x-first.x;
        const auto y_dist = last.y-first.y;
  
        return std::hypot(x_dist, y_dist);
    }
    
    constexpr auto distance(last_pos end, first_pos begin)
    {
        const position& first = std::move(begin);
        const position& last  = std::move(end);
        
        const auto x_dist = last.x-first.x;
        const auto y_dist = last.y-first.y;
        const auto z_dist = last.z-first.z;
  
        return std::hypot(x_dist, y_dist, z_dist);
    }

    //NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
    suite tagged_boundary_tests = [] mutable {
        "in-place construction and destructive extraction"_test = [] mutable {
            static_assert(!std::copy_constructible<move_only_t>, "`move_only_t` must be move-only, or this test is invalid.");
            static_assert(std::move_constructible<move_only_t>, "`move_only_t` must be move-only, or this test is invalid.");

            constexpr std::int32_t expected{42};
            using tagged_type = base::vocab::tagged_boundary<test_tag, move_only_t>;

            //Because `move_only_t` is noncopyable/move-only, successful extraction of the value implies the value was moved from the wrapper to the result.
            const move_only_t extracted = tagged_type{expected};

            expect(eq(extracted.value, expected));
        };
    
        "multi argument forwarding"_test = [] mutable {
            constexpr std::int32_t expected_x{10};
            constexpr std::int32_t expected_y{20};
            using point_tagged = base::vocab::tagged_boundary<test_tag, point>;

            const point p = point_tagged{expected_x, expected_y};
            expect(eq(p.x, expected_x));
            expect(eq(p.y, expected_y));
        };
    
        "type traits and immovability"_test = [] mutable {
            using bound_t = base::vocab::tagged_boundary<test_tag, std::int32_t>;

            expect(eq(std::is_copy_constructible_v<bound_t>, false));
            expect(eq(std::is_copy_assignable_v<bound_t>, false));
            expect(eq(std::is_move_constructible_v<bound_t>, false));
            expect(eq(std::is_move_assignable_v<bound_t>, false));
        };
    
        "rvalue-only conversion (ref-qualification contract)"_test = [] mutable {
            using bound_t = base::vocab::tagged_boundary<test_tag, std::int32_t>;

            expect(eq(std::convertible_to<bound_t, std::int32_t>, true));
            expect(eq(std::convertible_to<bound_t&, std::int32_t>, false));
            expect(eq(std::convertible_to<const bound_t&, std::int32_t>, false));
        };

        "distinct tags produce distinct types"_test = [] mutable {
            struct tag_a;
            struct tag_b;

            using bound_a = base::vocab::tagged_boundary<tag_a, std::int32_t>;
            using bound_b = base::vocab::tagged_boundary<tag_b, std::int32_t>;

            expect(eq(std::same_as<bound_a, bound_b>, false));
            expect(eq(std::constructible_from<bound_a, bound_b>, false));
            expect(eq(std::constructible_from<bound_b, bound_a>, false));
            expect(eq(std::convertible_to<bound_a, bound_b>, false));
            expect(eq(std::convertible_to<bound_b, bound_a>, false));
        };

        "explicit construction (no implicit wrapper creation)"_test = [] mutable {
            using bound_t = base::vocab::tagged_boundary<test_tag, std::int32_t>;

            expect(eq(std::constructible_from<bound_t, std::int32_t>, true));
            expect(eq(std::convertible_to<std::int32_t, bound_t>, false));
        };

        "constructor forwarding constraints protect deletion of copy and move constructors"_test = [] mutable {
            using bound_t = base::vocab::tagged_boundary<test_tag, std::int32_t>;

            expect(eq(std::constructible_from<bound_t, bound_t&>, false));
            expect(eq(std::constructible_from<bound_t, const bound_t&>, false));
            expect(eq(std::constructible_from<bound_t, bound_t>, false));
            expect(eq(std::constructible_from<bound_t, bound_t&&>, false));
        };
    
        "interface boundary type safety and unwrapping"_test = [] mutable {
            struct local_tag;
            struct remote_tag;

            struct target_class {
                using local_pred  = base::vocab::tagged_boundary<local_tag, std::function<bool()>>;
                using remote_pred = base::vocab::tagged_boundary<remote_tag, std::function<bool()>>;

                std::function<bool()> local_fn;
                std::function<bool()> remote_fn;

                // Pass-by-value and move idiom implicitly unwraps prvalue tagged_boundary
                target_class(local_pred local, remote_pred remote)
                    : local_fn(std::move(local))
                    , remote_fn(std::move(remote)) {}
            };

            target_class obj(
                target_class::local_pred{[]{ return true; }},
                target_class::remote_pred{[]{ return false; }}
            );

            expect(eq(obj.local_fn(), true));
            expect(eq(obj.remote_fn(), false));
        };

        "reference type support"_test = [] mutable {
            constexpr std::int32_t expected_original{100};
            constexpr std::int32_t expected_changed{200};

            std::int32_t original = expected_original;
            using ref_t = base::vocab::tagged_boundary<test_tag, std::int32_t&>;

            auto bind_ref = [](ref_t tagged) -> std::int32_t& {
                return std::move(tagged);
            };

            std::int32_t& bound_ref = bind_ref(ref_t{original});

            expect(eq(bound_ref, expected_original));

            bound_ref = expected_changed;

            expect(eq(original, expected_changed));
            expect(eq(&bound_ref == &original, true));
        };

        "noexcept specification propagation"_test = [] mutable {
            struct throw_on_move {
                throw_on_move() = default;
                throw_on_move(throw_on_move&&) noexcept(false) {}
            };

            struct noexcept_move {
                noexcept_move() = default;
                noexcept_move(noexcept_move&&) noexcept(true) {}
            };

            using throwing_boundary = base::vocab::tagged_boundary<test_tag, throw_on_move>;
            using noexcept_boundary = base::vocab::tagged_boundary<test_tag, noexcept_move>;

            expect(eq(noexcept(static_cast<throw_on_move>(std::declval<throwing_boundary>())), false));
            expect(eq(noexcept(static_cast<noexcept_move>(std::declval<noexcept_boundary>())), true));
        };

        "2d point value boundary distance"_test = [] mutable {
            constexpr point p1{0, 0};
            constexpr point p2{3, 4};
            constexpr double expected{5.0};

            const auto result = distance(first_point{p1}, last_point{p2});
            expect(eq(result, expected)(0.01));
        };

        "3d position reference boundary distance"_test = [] mutable {
            constexpr position pos1{position::longitude_t{0}, position::elevation_t{0}, position::latitude_t{0}};
            constexpr position pos2{position::longitude_t{3}, position::elevation_t{4}, position::latitude_t{12}};
            constexpr double expected{13.0};

            // Parameter order in function signature is (last_pos, first_pos),
            // but strong boundary types make call sites explicit and safe.
            const auto result = distance(last_pos{pos2}, first_pos{pos1});
            expect(eq(result, expected)(0.01));
        };
    };
} //namespace

int main() {}
