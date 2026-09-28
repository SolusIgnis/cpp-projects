// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:options

import net.telnet;
import boost.ut;
import std;

using namespace boost::ext::ut;

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    using net::telnet::option;
    using net::telnet::negotiation_direction;
    using net::telnet::byte_t;

    //=============================================================
    // option::id_num enum structural properties
    //=============================================================

    "option::id_num enum structural guarantees"_test = [] mutable {
        expect(std::is_enum_v<option::id_num>);
        expect(std::is_same_v<std::underlying_type_t<option::id_num>, byte_t>);
    };

    //=============================================================
    // option tests
    //=============================================================

    "option stores id and name"_test = [] mutable {
        const option opt{option::id_num::echo, "Echo"};

        expect(eq(opt.get_id(), option::id_num::echo));
        expect(eq(opt.get_name(), std::string{"Echo"}));
    };

    "default predicates reject"_test = [] mutable {
        const option opt{option::id_num::echo};

        expect(!opt.supports_local());
        expect(!opt.supports_remote());
    };

    "always_accept predicate works"_test = [] mutable {
        const option opt{option::id_num::echo, "Echo", option::local_predicate{option::always_accept}, option::remote_predicate{option::always_accept}};

        expect(opt.supports_local());
        expect(opt.supports_remote());
    };

    "supports(direction) dispatches correctly"_test = [] mutable {
        const option opt{option::id_num::echo, "Echo", option::local_predicate{option::always_accept}, option::remote_predicate{option::always_reject}};

        expect(opt.supports(negotiation_direction::local));
        expect(!opt.supports(negotiation_direction::remote));
    };

    "make_option sets predicates from booleans"_test = [] mutable {
        const auto opt = option::make_option(option::id_num::binary, "Binary", true, false);

        expect(opt.supports_local());
        expect(!opt.supports_remote());
    };

    "subnegotiation defaults"_test = [] mutable {
        constexpr std::size_t subnegotiation_limit{1024};
        const option opt{option::id_num::binary};

        expect(!opt.supports_subnegotiation());
        expect(eq(opt.max_subnegotiation_size(), subnegotiation_limit));
    };

    "subnegotiation configuration honored"_test = [] mutable {
        constexpr std::size_t subnegotiation_limit{4096};
        const option opt{
            option::id_num::binary,
            "Binary",
            option::local_predicate{option::always_accept},
            option::remote_predicate{option::always_accept},
            true,
            static_cast<std::size_t>(subnegotiation_limit)
        };

        expect(opt.supports_subnegotiation());
        expect(eq(opt.max_subnegotiation_size(), subnegotiation_limit));
    };

    "three-way comparison based on id"_test = [] mutable {
        const option opt_a{option::id_num::binary};
        const option opt_b{option::id_num::echo};

        expect(that % opt_a < opt_b);
        expect(that % opt_b > opt_a);
        expect(that % opt_a == option::id_num::binary);
    };

    "implicit conversion to id_num works"_test = [] mutable {
        const option opt{option::id_num::echo};
        const option::id_num id = opt;

        expect(that % id == option::id_num::echo);
    };
}
