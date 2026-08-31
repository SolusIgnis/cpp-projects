// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
// Unit tests for net.telnet:options

import net.telnet;
import ut;
import std;

using namespace ut;
using namespace net::telnet;
using namespace std::literals;

namespace {
    constexpr inline std::size_t default_max_subnegotiation_buffer_size{1024};
    //NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
    suite telnet_options_tests = [] mutable {
        using net::telnet::byte_t;

        //NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers): Verifying literal values.
        "option::id_num enum values (selected)"_test = [] mutable {
            expect(eq(std::to_underlying(option::id_num::binary), static_cast<byte_t>(0x00)));
            expect(eq(std::to_underlying(option::id_num::echo), static_cast<byte_t>(0x01)));
            expect(eq(std::to_underlying(option::id_num::suppress_go_ahead), static_cast<byte_t>(0x03)));
            expect(eq(std::to_underlying(option::id_num::terminal_type), static_cast<byte_t>(0x18)));
            expect(eq(std::to_underlying(option::id_num::linemode), static_cast<byte_t>(0x22)));
            expect(eq(std::to_underlying(option::id_num::mccp2), static_cast<byte_t>(0x56)));
            expect(eq(std::to_underlying(option::id_num::gmcp), static_cast<byte_t>(0xC9)));
            expect(eq(std::to_underlying(option::id_num::extended_options_list), static_cast<byte_t>(0xFF)));
        };
        //NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

        "option default construction and accessors"_test = [] mutable {
            constexpr auto default_name{"Echo"s};

            const option opt{option::id_num::echo, default_name};
            expect(eq(opt.get_id(), option::id_num::echo));
            expect(eq(opt.get_name(), default_name));
            expect(eq(opt.supports_local(), false));
            expect(eq(opt.supports_remote(), false));
            expect(eq(opt.supports(negotiation_direction::local), false));
            expect(eq(opt.supports(negotiation_direction::remote), false));
            expect(eq(opt.supports_subnegotiation(), false));
            expect(eq(opt.max_subnegotiation_size(), default_max_subnegotiation_buffer_size));
        };

        "option with custom predicates and subnegotiation"_test = [] mutable {
            constexpr auto opt_name{"Terminal Type"};
            constexpr std::size_t buffer_size{512};

            const auto always_on = [](option::id_num) { return true; };
            const option opt{option::id_num::terminal_type, opt_name, always_on, always_on, true, buffer_size};

            expect(eq(opt.supports_local(), true));
            expect(eq(opt.supports_remote(), true));
            expect(eq(opt.supports(negotiation_direction::local), true));
            expect(eq(opt.supports(negotiation_direction::remote), true));
            expect(eq(opt.supports_subnegotiation(), true));
            expect(eq(opt.max_subnegotiation_size(), buffer_size));
        };

        "make_option factory"_test = [] mutable {
            constexpr auto opt_name1{"Echo"s};
            constexpr auto opt_name2{"Linemode"s};
            constexpr std::size_t buffer_size{256};

            const auto opt1 = option::make_option(option::id_num::echo, opt_name1, true, false);
            expect(eq(opt1.get_id(), option::id_num::echo));
            expect(eq(opt1.get_name(), opt_name1));
            expect(eq(opt1.supports_local(), true));
            expect(eq(opt1.supports_remote(), false));
            expect(eq(opt1.supports_subnegotiation(), false));

            const auto opt2 =
                option::make_option(option::id_num::linemode, opt_name2, false, true, true, buffer_size);
            expect(eq(opt2.supports_local(), false));
            expect(eq(opt2.supports_remote(), true));
            expect(eq(opt2.supports_subnegotiation(), true));
            expect(eq(opt2.max_subnegotiation_size(), buffer_size));
        };

        "option always_accept / always_reject predicates"_test = [] mutable {
            expect(eq(option::always_accept(option::id_num::echo), true));
            expect(eq(option::always_reject(option::id_num::echo), false));
        };

        "option three-way comparison (same type)"_test = [] mutable {
            const option echo1{option::id_num::echo, "Echo"s};
            const option sga{option::id_num::suppress_go_ahead, "SGA"s};
            const option echo2{option::id_num::echo, "Echo again"s};

            expect(eq((echo1 <=> sga) == std::strong_ordering::less, true));
            expect(eq((sga <=> echo1) == std::strong_ordering::greater, true));
            expect(eq((echo1 <=> echo2) == std::strong_ordering::equal, true));
        };

        "option comparison with id_num"_test = [] mutable {
            const option opt{option::id_num::terminal_type};
            expect(eq((opt <=> option::id_num::echo) == std::strong_ordering::greater, true));
            expect(eq((opt <=> option::id_num::terminal_type) == std::strong_ordering::equal, true));
            expect(eq((opt <=> option::id_num::linemode) == std::strong_ordering::less, true));
        };

        "option implicit conversion to id_num"_test = [] mutable {
            const option opt{option::id_num::gmcp};
            const option::id_num id = opt; // implicit
            expect(eq(id, option::id_num::gmcp));
            expect(eq(opt == option::id_num::gmcp, true));
            expect(eq(opt != option::id_num::mccp2, true));
        };

        "option_registry default empty"_test = [] mutable {
            const option_registry reg{};
            expect(eq(reg.has(option::id_num::echo), false));
            expect(eq(reg.get(option::id_num::echo).has_value(), false));
        };

        "option_registry initializer_list construction"_test = [] mutable {
            constexpr auto echo_name{"Echo"s};
            constexpr auto sga_name{"SGA"s};
            constexpr auto linemode_name{"Linemode"};

            const option_registry reg{
                option::make_option(option::id_num::echo, echo_name, true, true),
                option::make_option(option::id_num::suppress_go_ahead, sga_name, true, false),
                option::make_option(option::id_num::linemode, linemode_name, false, true, true),
            };

            expect(eq(reg.has(option::id_num::echo), true));
            expect(eq(reg.has(option::id_num::terminal_type), false));

            const auto maybe_echo = reg.get(option::id_num::echo);
            expect(eq(maybe_echo.has_value(), true));
            expect(eq(maybe_echo->get_name(), echo_name));
            expect(eq(maybe_echo->supports_local(), true));
            expect(eq(maybe_echo->supports_remote(), true));
        };

        "option_registry upsert"_test = [] mutable {
            constexpr auto name1{"Binary"s};
            constexpr auto name2{"Binary Transmission"s};

            option_registry reg{};

            // Insert new
            const auto& inserted = reg.upsert(option::make_option(option::id_num::binary, name1, true, true));
            expect(eq(inserted.get_name(), name1));
            expect(eq(reg.has(option::id_num::binary), true));

            // Update existing
            reg.upsert(option::make_option(option::id_num::binary, name2, false, true));
            const auto updated = reg.get(option::id_num::binary);
            expect(eq(updated.has_value(), true));
            if (updated) {
                expect(eq(updated->get_name(), name2));
                expect(eq(updated->supports_local(), false));
            }
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
            constexpr auto opt_name{"Charset"s};
            constexpr std::size_t buffer_size{128};

            option_registry reg{};
            const auto& opt =
                reg.upsert(option::id_num::charset, opt_name, option::always_accept, option::always_reject, true, buffer_size);
            expect(eq(opt.get_name(), opt_name));
            expect(eq(opt.supports_local(), true));
            expect(eq(opt.supports_remote(), false));
            expect(eq(opt.supports_subnegotiation(), true));
            expect(eq(opt.max_subnegotiation_size(), buffer_size));
        };

        //NOLINTBEGIN(bugprone-argument-comment): Matcher lhs/rhs.
        "option formatter default 'd'"_test = [] mutable {
            const option opt{option::id_num::echo, "Echo"};
            expect(eq(std::format("{}", opt), "0x01 (Echo)"s));

            const option unnamed{option::id_num::xauth};
            expect(eq(std::format("{}", unnamed), "0x29 (unknown)"s));
        };
        //NOLINTEND(bugprone-argument-comment)

        //NOLINTBEGIN(bugprone-argument-comment): Matcher lhs/rhs.
        "option formatter 'n'"_test = [] mutable {
            const option opt{option::id_num::linemode, "Linemode"};
            expect(eq(std::format("{:n}", opt), "Linemode"s));

            const option unnamed{option::id_num::mcp};
            expect(eq(std::format("{:n}", unnamed), "unknown"s));
        };
        //NOLINTEND(bugprone-argument-comment)

        //NOLINTBEGIN(bugprone-argument-comment): Matcher lhs/rhs.
        "option formatter 'x'"_test = [] mutable {
            const option opt{option::id_num::gmcp};
            expect(eq(std::format("{:x}", opt), "0xc9"s));
        };
        //NOLINTEND(bugprone-argument-comment)
    };
} //namespace

int main() {}
