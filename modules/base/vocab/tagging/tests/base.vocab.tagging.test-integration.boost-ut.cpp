// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

import base.vocab.tagging;
import boost.ut;
import std;

using namespace boost::ext::ut;

namespace {
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
            : x(std::move(x_val)), y(std::move(y_val)), z(std::move(z_val))
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

        const auto x_dist = last.x - first.x;
        const auto y_dist = last.y - first.y;

        return std::hypot(x_dist, y_dist);
    }

    constexpr auto distance(last_pos end, first_pos begin)
    {
        const position& first = std::move(begin);
        const position& last  = std::move(end);

        const auto x_dist = last.x - first.x;
        const auto y_dist = last.y - first.y;
        const auto z_dist = last.z - first.z;

        return std::hypot(x_dist, y_dist, z_dist);
    }
} //namespace

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    "2d point value boundary distance"_test = [] mutable {
        constexpr point pt1{0, 0}; //NOLINT(bugprone-argument-comment)
        constexpr point pt2{3, 4}; //NOLINT(bugprone-argument-comment)
        constexpr double expected{5.0};

        const auto result = distance(first_point{pt1}, last_point{pt2});
        expect(eq(result, expected));
    };

    "3d position reference boundary distance"_test = [] mutable {
        constexpr position pos1{position::longitude_t{0}, position::elevation_t{0}, position::latitude_t{0}};
        constexpr position pos2{position::longitude_t{3}, position::elevation_t{4}, position::latitude_t{12}};
        constexpr double expected{13.0};

        // Parameter order in function signature is (last_pos, first_pos),
        // but strong boundary types make call sites explicit and safe.
        const auto result = distance(last_pos{pos2}, first_pos{pos1});
        expect(eq(result, expected));
    };
}
