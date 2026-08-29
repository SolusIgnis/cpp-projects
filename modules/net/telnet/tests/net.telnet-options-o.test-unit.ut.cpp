// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:options

import net.telnet;
import ut;
import std;

using namespace ut;

namespace {
    //NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
    suite net_telnet_option_unit_tests = [] mutable {
        using net::telnet::option;
        using net::telnet::negotiation_direction;
        using net::telnet::byte_t;

        // ============================================================
        // option::id_num enum structural properties
        // ============================================================

        "option::id_num enum structural guarantees"_test = [] mutable {
            expect(eq(std::is_enum_v<option::id_num>, true));
            expect(eq(std::is_same_v<std::underlying_type_t<option::id_num>, byte_t>, true));
        };

        // ============================================================
        // option tests
        // ============================================================

        "option stores id and name"_test = [] mutable {
            option opt{option::id_num::echo, "Echo"};

            expect(eq((opt.get_id() == option::id_num::echo), true));
            expect(eq(opt.get_name(), std::string{"Echo"}));
        };

        "default predicates reject"_test = [] mutable {
            option opt{option::id_num::echo};

            expect(eq(opt.supports_local(), false));
            expect(eq(opt.supports_remote(), false));
        };

        "always_accept predicate works"_test = [] mutable {
            option opt{option::id_num::echo, "Echo", option::always_accept, option::always_accept};

            expect(eq(opt.supports_local(), true));
            expect(eq(opt.supports_remote(), true));
        };

        "supports(direction) dispatches correctly"_test = [] mutable {
            option opt{option::id_num::echo, "Echo", option::always_accept, option::always_reject};

            expect(eq(opt.supports(negotiation_direction::local), true));
            expect(eq(opt.supports(negotiation_direction::remote), false));
        };

        "make_option sets predicates from booleans"_test = [] mutable {
            auto opt = option::make_option(option::id_num::binary, "Binary", true, false);

            expect(eq(opt.supports_local(), true));
            expect(eq(opt.supports_remote(), false));
        };

        "subnegotiation defaults"_test = [] mutable {
            option opt{option::id_num::binary};

            expect(eq(opt.supports_subnegotiation(), false));
            expect(eq(opt.max_subnegotiation_size(), static_cast<std::size_t>(1024)));
        };

        "subnegotiation configuration honored"_test = [] mutable {
            option
                opt{option::id_num::binary,
                    "Binary",
                    option::always_accept,
                    option::always_accept,
                    true,
                    static_cast<std::size_t>(4096)};

            expect(eq(opt.supports_subnegotiation(), true));
            expect(eq(opt.max_subnegotiation_size(), static_cast<std::size_t>(4096)));
        };

        "three-way comparison based on id"_test = [] mutable {
            option a{option::id_num::binary};
            option b{option::id_num::echo};

            expect(eq((a < b), true));
            expect(eq((b > a), true));
            expect(eq((a == option::id_num::binary), true));
        };

        "implicit conversion to id_num works"_test = [] mutable {
            option opt{option::id_num::echo};
            option::id_num id = opt;

            expect(eq((id == option::id_num::echo), true));
        };
    };
} //namespace

int main() {}
