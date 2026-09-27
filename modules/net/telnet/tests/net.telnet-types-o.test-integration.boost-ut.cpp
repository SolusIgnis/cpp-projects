// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:types formatting

import net.telnet;
import boost.ut;
import std;

using namespace boost::ext::ut;
using namespace std::literals;

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    using net::telnet::command;
    using net::telnet::negotiation_direction;

    //------------------------------------------------------------
    // command default formatting
    //------------------------------------------------------------

    "default format produces name and hex"_test = [] mutable {
        const auto formatted = std::format("{}", command::will_opt);
        expect(eq(formatted, "WILL (0xFB)"s));
    };

    //------------------------------------------------------------
    // name-only formatting
    //------------------------------------------------------------

    "name-only formatting works"_test = [] mutable {
        const auto formatted = std::format("{:n}", command::brk);
        expect(eq(formatted, "BRK"s));
    };

    //------------------------------------------------------------
    // hex-only formatting
    //------------------------------------------------------------

    "hex-only formatting works"_test = [] mutable {
        const auto formatted1 = std::format("{:x}", command::ec);
        expect(eq(formatted1, "0xf7"s));

        const auto formatted2 = std::format("{:X}", command::ec);
        expect(eq(formatted2, "0xF7"s));
    };

    //------------------------------------------------------------
    // unknown command formatting
    //------------------------------------------------------------

    "unknown command formats as UNKNOWN in name mode"_test = [] mutable {
        constexpr auto invalid_command_num{0x0A};
        //NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
        const auto unknown   = static_cast<command>(invalid_command_num);
        const auto formatted = std::format("{:n}", unknown);
        expect(eq(formatted, "UNKNOWN"s));
    };

    "unknown command formats as hex in hex mode"_test = [] mutable {
        constexpr auto invalid_command_num{0x0A};
        const auto unknown = static_cast<command>(invalid_command_num); //NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
        const auto formatted1 = std::format("{:x}", unknown);
        expect(eq(formatted1, "0x0a"s));
        const auto formatted2 = std::format("{:X}", unknown);
        expect(eq(formatted2, "0x0A"s));
    };

    //------------------------------------------------------------
    // invalid format specifier throws
    //------------------------------------------------------------

    "invalid command format specifier throws"_test = [] mutable {
        const auto thing_to_format = command::iac;
        expect(throws<std::format_error>([=]{ [[maybe_unused]] const auto formatted = std::vformat("{:z}", std::make_format_args(thing_to_format)); }));
    };

    //------------------------------------------------------------
    // negotiation_direction formatting
    //------------------------------------------------------------

    "negotiation_direction formats correctly"_test = [] mutable {
        const auto formatted1 = std::format("{}", negotiation_direction::local);
        const auto formatted2 = std::format("{}", negotiation_direction::remote);

        expect(eq(formatted1, "local"s));
        expect(eq(formatted2, "remote"s));
    };

    "invalid negotiation_direction format throws"_test = [] mutable {
        const auto thing_to_format = negotiation_direction::local;
        expect(throws<std::format_error>([=]{ [[maybe_unused]] const auto formatted = std::vformat("{:z}", std::make_format_args(thing_to_format)); }));
    };

    //============================================================
    // Composability inside larger formatted expressions
    //============================================================

    "formatter composes inside nested format expressions"_test = [] mutable {
        const auto msg = std::format("[{}:{}]", command::iac, negotiation_direction::remote);
        expect(eq(msg, "[IAC (0xFF):remote]"s));
    };
}
