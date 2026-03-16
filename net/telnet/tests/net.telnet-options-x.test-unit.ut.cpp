// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
// Unit tests for net.telnet:options

import net.telnet;
import ut;
import std;

using namespace ut;
using namespace net::telnet;

suite telnet_options_tests = [] mutable {

    "option::id_num enum values (selected)"_test = [] mutable {
        expect(eq(std::to_underlying(option::id_num::binary),                 0x00));
        expect(eq(std::to_underlying(option::id_num::echo),                   0x01));
        expect(eq(std::to_underlying(option::id_num::suppress_go_ahead),      0x03));
        expect(eq(std::to_underlying(option::id_num::terminal_type),          0x18));
        expect(eq(std::to_underlying(option::id_num::linemode),               0x22));
        expect(eq(std::to_underlying(option::id_num::mccp2),                  0x56));
        expect(eq(std::to_underlying(option::id_num::gmcp),                   0xC9));
        expect(eq(std::to_underlying(option::id_num::extended_options_list),  0xFF));
    };

    "option default construction and accessors"_test = [] mutable {
        option opt{option::id_num::echo, "Echo"};
        expect(eq(opt.get_id(), option::id_num::echo));
        expect(eq(opt.get_name(), "Echo"));
        expect(eq(opt.supports_local(), false));
        expect(eq(opt.supports_remote(), false));
        expect(eq(opt.supports(negotiation_direction::local), false));
        expect(eq(opt.supports(negotiation_direction::remote), false));
        expect(eq(opt.supports_subnegotiation(), false));
        expect(eq(opt.max_subnegotiation_size(), 1024uz));
    };

    "option with custom predicates and subnegotiation"_test = [] mutable {
        auto always_on = [](option::id_num) { return true; };
        option opt{
            option::id_num::terminal_type,
            "Terminal Type",
            always_on,
            always_on,
            true,
            512
        };

        expect(eq(opt.supports_local(), true));
        expect(eq(opt.supports_remote(), true));
        expect(eq(opt.supports(negotiation_direction::local), true));
        expect(eq(opt.supports(negotiation_direction::remote), true));
        expect(eq(opt.supports_subnegotiation(), true));
        expect(eq(opt.max_subnegotiation_size(), 512uz));
    };

    "make_option factory"_test = [] mutable {
        auto opt1 = option::make_option(option::id_num::echo, "Echo", true, false);
        expect(eq(opt1.get_id(), option::id_num::echo));
        expect(eq(opt1.get_name(), "Echo"));
        expect(eq(opt1.supports_local(), true));
        expect(eq(opt1.supports_remote(), false));
        expect(eq(opt1.supports_subnegotiation(), false));

        auto opt2 = option::make_option(option::id_num::linemode, "Linemode", false, true, true, 256);
        expect(eq(opt2.supports_local(), false));
        expect(eq(opt2.supports_remote(), true));
        expect(eq(opt2.supports_subnegotiation(), true));
        expect(eq(opt2.max_subnegotiation_size(), 256uz));
    };

    "option always_accept / always_reject predicates"_test = [] mutable {
        expect(eq(option::always_accept(option::id_num::echo), true));
        expect(eq(option::always_reject(option::id_num::echo), false));
    };

    "option three-way comparison (same type)"_test = [] mutable {
        option a{option::id_num::echo, "Echo"};
        option b{option::id_num::suppress_go_ahead, "SGA"};
        option c{option::id_num::echo, "Echo again"};

        expect(eq(a <=> b, std::strong_ordering::less));
        expect(eq(b <=> a, std::strong_ordering::greater));
        expect(eq(a <=> c, std::strong_ordering::equal));
    };

    "option comparison with id_num"_test = [] mutable {
        option opt{option::id_num::terminal_type};
        expect(eq(opt <=> option::id_num::echo, std::strong_ordering::greater));
        expect(eq(opt <=> option::id_num::terminal_type, std::strong_ordering::equal));
        expect(eq(opt <=> option::id_num::linemode, std::strong_ordering::less));
    };

    "option implicit conversion to id_num"_test = [] mutable {
        option opt{option::id_num::gmcp};
        option::id_num id = opt;  // implicit
        expect(eq(id, option::id_num::gmcp));
        expect(eq(opt == option::id_num::gmcp, true));
        expect(eq(opt != option::id_num::mccp2, true));
    };

    "option_registry default empty"_test = [] mutable {
        option_registry reg{};
        expect(eq(reg.has(option::id_num::echo), false));
        expect(eq(reg.get(option::id_num::echo).has_value(), false));
    };

    "option_registry initializer_list construction"_test = [] mutable {
        option_registry reg{
            option::make_option(option::id_num::echo, "Echo", true, true),
            option::make_option(option::id_num::suppress_go_ahead, "SGA", true, false),
            option::make_option(option::id_num::linemode, "Linemode", false, true, true)
        };

        expect(eq(reg.has(option::id_num::echo), true));
        expect(eq(reg.has(option::id_num::terminal_type), false));

        auto maybe_echo = reg.get(option::id_num::echo);
        expect(eq(maybe_echo.has_value(), true));
        expect(eq(maybe_echo->get_name(), "Echo"));
        expect(eq(maybe_echo->supports_local(), true));
        expect(eq(maybe_echo->supports_remote(), true));
    };

    "option_registry upsert"_test = [] mutable {
        option_registry reg{};

        // Insert new
        const auto& inserted = reg.upsert(option::make_option(option::id_num::binary, "Binary", true, true));
        expect(eq(inserted.get_name(), "Binary"));
        expect(eq(reg.has(option::id_num::binary), true));

        // Update existing
        reg.upsert(option::make_option(option::id_num::binary, "Binary Transmission", false, true));
        auto updated = reg.get(option::id_num::binary);
        expect(eq(updated.has_value(), true));
        expect(eq(updated->get_name(), "Binary Transmission"));
        expect(eq(updated->supports_local(), false));
    };

    "option_registry upsert with ec"_test = [] mutable {
        option_registry reg{};
        std::error_code ec;

        reg.upsert(option::make_option(option::id_num::status, "Status"), ec);
        expect(eq(!ec, true));

        // Update to test error path coverage (unlikely to fail here, but exercises the try-catch)
        reg.upsert(option::make_option(option::id_num::status, "Status Updated", true, true), ec);
        expect(eq(!ec, true));
    };

    "option_registry upsert variadic"_test = [] mutable {
        option_registry reg{};
        const auto& opt = reg.upsert(option::id_num::charset, "Charset", true, false, true, 128);
        expect(eq(opt.get_name(), "Charset"));
        expect(eq(opt.supports_local(), true));
        expect(eq(opt.supports_remote(), false));
        expect(eq(opt.supports_subnegotiation(), true));
        expect(eq(opt.max_subnegotiation_size(), 128uz));
    };

    "option formatter default 'd'"_test = [] mutable {
        option opt{option::id_num::echo, "Echo"};
        expect(eq(std::format("{}", opt), "0x01 (Echo)"));

        option unnamed{option::id_num::xauth};
        expect(eq(std::format("{}", unnamed), "0x29 (unknown)"));
    };

    "option formatter 'n'"_test = [] mutable {
        option opt{option::id_num::linemode, "Linemode"};
        expect(eq(std::format("{:n}", opt), "Linemode"));

        option unnamed{option::id_num::mcp};
        expect(eq(std::format("{:n}", unnamed), "unknown"));
    };

    "option formatter 'x'"_test = [] mutable {
        option opt{option::id_num::gmcp};
        expect(eq(std::format("{:x}", opt), "0xc9"));
    };
};

int main() {}
