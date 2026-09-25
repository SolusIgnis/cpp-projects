// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

import base.vocab.tagging;
import boost.ut;
import std;

using namespace boost::ext::ut;

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    struct test_tag;

    "in-place construction and destructive extraction"_test = [] mutable {
        //Move-only target type used to verify destructive extraction
        struct move_only_t {
            std::int32_t value{0};

            constexpr explicit move_only_t(std::int32_t val) : value(val) {}

            constexpr ~move_only_t() = default;

            constexpr move_only_t(move_only_t&& other) noexcept            = default;
            constexpr move_only_t& operator=(move_only_t&& other) noexcept = default;

            constexpr move_only_t(const move_only_t&)            = delete;
            constexpr move_only_t& operator=(const move_only_t&) = delete;
        };

        static_assert(!std::copy_constructible<move_only_t>, "`move_only_t` must be move-only, or this test is invalid.");
        static_assert(std::move_constructible<move_only_t>, "`move_only_t` must be move-only, or this test is invalid.");

        constexpr std::int32_t expected{42};
        using tagged_type = base::vocab::tagged_boundary<test_tag, move_only_t>;

        //Because `move_only_t` is noncopyable/move-only, successful extraction of the value implies the value was moved from the wrapper to the result.
        const move_only_t extracted = tagged_type{expected};

        expect(eq(extracted.value, expected));
    };

    "multi argument forwarding"_test = [] mutable {
        struct point {
            std::int32_t x;
            std::int32_t y;
    
            constexpr point(std::int32_t x_val, std::int32_t y_val) : x(x_val), y(y_val) {}
        };

        constexpr std::int32_t expected_x{10};
        constexpr std::int32_t expected_y{20};
        using point_tagged = base::vocab::tagged_boundary<test_tag, point>;

        constexpr point ptt = point_tagged{expected_x, expected_y};
        expect(eq(ptt.x, expected_x));
        expect(eq(ptt.y, expected_y));
    };

    "move_constructible but noncopyable and non-assignable"_test = [] mutable {
        using bound_t = base::vocab::tagged_boundary<test_tag, std::int32_t>;

        expect(that % !std::is_copy_constructible_v<bound_t>) << "non-copy-constructible";
        expect(that % !std::is_copy_assignable_v<bound_t>) << "non-copy-assignable";
        expect(that % std::is_move_constructible_v<bound_t>) << "move constructible";
        expect(that % !std::is_move_assignable_v<bound_t>) << "non-move-assignable";
    };

    "rvalue-only conversion (ref-qualification contract)"_test = [] mutable {
        using bound_t = base::vocab::tagged_boundary<test_tag, std::int32_t>;

        expect(that % std::convertible_to<bound_t, std::int32_t>) << "prvalue convertible to underlying";
        expect(that % std::convertible_to<bound_t&&, std::int32_t>) << "xvalue convertible to underlying";
        expect(that % !std::convertible_to<bound_t&, std::int32_t>) << "mutable lvalue not convertible to underlying";
        expect(that % !std::convertible_to<const bound_t&, std::int32_t>) << "const lvalue not convertible to underlying";
    };

    "distinct tags produce distinct types"_test = [] mutable {
        struct tag_a;
        struct tag_b;

        using bound_a = base::vocab::tagged_boundary<tag_a, std::int32_t>;
        using bound_b = base::vocab::tagged_boundary<tag_b, std::int32_t>;

        //Tags participate in the type, so the type is not the same.
        expect(that % !std::same_as<bound_a, bound_b>);

        //`tagged_boundary` converts to its underlying type which can explicitly construct a `tagged_boundary` with a different tag.
        expect(that % std::constructible_from<bound_a, bound_b>);
        expect(that % std::constructible_from<bound_b, bound_a>);

        //A `tagged_boundary` with one tag cannot implicitly convert to a `tagged_boundary` with a different tag.
        expect(that % !std::convertible_to<bound_a, bound_b>);
        expect(that % !std::convertible_to<bound_b, bound_a>);
    };

    "explicit construction (no implicit wrapper creation)"_test = [] mutable {
        using bound_t = base::vocab::tagged_boundary<test_tag, std::int32_t>;

        expect(that % std::constructible_from<bound_t, std::int32_t>) << "(explicitly) constructible from underlying";
        expect(that % !std::convertible_to<std::int32_t, bound_t>) << "not (implicitly) convertible from underlying";
    };

    "constructor constraints and move construction"_test = [] mutable {
        using bound_t = base::vocab::tagged_boundary<test_tag, std::int32_t>;

        expect(that % !std::constructible_from<bound_t, bound_t&>) << "not copyable from mutable lvalue";
        expect(that % !std::constructible_from<bound_t, const bound_t&>) << "not copyable from const lvalue";
        expect(that % std::constructible_from<bound_t, bound_t>) << "movable from prvalue";
        expect(that % std::constructible_from<bound_t, bound_t&&>) << "movable from xvalue";
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
            target_class(local_pred local, remote_pred remote) : local_fn(std::move(local)), remote_fn(std::move(remote)) {}
        };

        const target_class obj(target_class::local_pred{[] { return true; }}, target_class::remote_pred{[] { return false; }});

        expect(that % obj.local_fn());
        expect(that % !obj.remote_fn());
    };

    "reference type support"_test = [] mutable {
        constexpr std::int32_t expected_original{100};
        constexpr std::int32_t expected_changed{200};

        std::int32_t original = expected_original;

        using ref_t = base::vocab::tagged_boundary<test_tag, std::int32_t&>;

        const auto bind_ref = [](ref_t tagged) -> std::int32_t& { return std::move(tagged); };

        std::int32_t& bound_ref = bind_ref(ref_t{original});

        expect(eq(bound_ref, expected_original));

        bound_ref = expected_changed;

        expect(eq(original, expected_changed)) << "values match";
        expect(eq(std::addressof(bound_ref), std::addressof(original))) << "addresses match";
    };

    "noexcept specification propagation"_test = [] mutable {
        //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
        struct throw_on_move {
            throw_on_move() = default;

            //NOLINTNEXTLINE(bugprone-unsafe-to-allow-exceptions): Trivial fixture testing noexcept specification.
            throw_on_move(throw_on_move&& /*unused*/) noexcept(false) {}
        };

        //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
        struct noexcept_move {
            noexcept_move() = default;
            noexcept_move(noexcept_move&& /*unused*/) noexcept(true) {}
        };

        using throwing_boundary = base::vocab::tagged_boundary<test_tag, throw_on_move>;
        using noexcept_boundary = base::vocab::tagged_boundary<test_tag, noexcept_move>;

        expect(that % !noexcept(static_cast<throw_on_move>(std::declval<throwing_boundary>())));
        expect(that % noexcept(static_cast<noexcept_move>(std::declval<noexcept_boundary>())));
    };

    "moving and forwarding"_test = [] mutable {
        using tagged_t = base::vocab::tagged_boundary<test_tag, std::int32_t>;

        //Extract and use the value.
        constexpr auto consumer = [](tagged_t arg) {
            std::int32_t val = std::move(arg);
            return val * val;
        };

        //Move the wrapper holding the value.
        constexpr auto mover = [consumer](tagged_t arg) { return consumer(std::move(arg)); };

        //Forward the wrapper holding the value.
        constexpr auto forwarder = [mover](auto&& arg) { return mover(std::forward<decltype(arg)>(arg)); };

        constexpr std::int32_t value{7};
        constexpr std::int32_t expected{49};

        constexpr auto result = forwarder(tagged_t{value});
        expect(eq(result, expected));
    };

    "perfect forwarding through variadic parameter pack"_test = [] mutable {
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
                : x(std::move(x_val)), y(std::move(y_val)), z(std::move(z_val))
            {}
        };

        struct begin_tag;
        struct end_tag;

        using first_pos   = base::vocab::tagged_boundary<begin_tag, const position&>;
        using last_pos    = base::vocab::tagged_boundary<end_tag, const position&>;

        constexpr auto forwarding_test = [](auto&&... args) { return distance(std::forward<decltype(args)>(args)...); };

        constexpr position pos1{position::longitude_t{-2}, position::elevation_t{10}, position::latitude_t{5}};
        constexpr position pos2{position::longitude_t{-1}, position::elevation_t{14}, position::latitude_t{13}};
        constexpr double expected{9.0};

        const auto result = forwarding_test(last_pos{pos2}, first_pos{pos1});
        expect(eq(result, expected));
    };
}
