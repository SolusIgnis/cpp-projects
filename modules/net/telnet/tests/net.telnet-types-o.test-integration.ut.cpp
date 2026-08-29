// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:types formatting

import net.telnet;
import ut;
import std;

using namespace ut;

namespace {
    //NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
    suite net_telnet_types_format_tests = [] mutable {
        using net::telnet::command;
        using net::telnet::negotiation_direction;

        // ------------------------------------------------------------
        // command default formatting
        // ------------------------------------------------------------

        "default format produces name and hex"_test = [] mutable {
            auto s = std::format("{}", command::will_opt);
            expect(eq(s, std::string{"WILL (0xFB)"}));
        };

        // ------------------------------------------------------------
        // name-only formatting
        // ------------------------------------------------------------

        "name-only formatting works"_test = [] mutable {
            auto s = std::format("{:n}", command::brk);
            expect(eq(s, std::string{"BRK"}));
        };

        // ------------------------------------------------------------
        // hex-only formatting
        // ------------------------------------------------------------

        "hex-only formatting works"_test = [] mutable {
            auto s1 = std::format("{:x}", command::ec);
            expect(eq(s1, std::string{"0xf7"}));

            auto s2 = std::format("{:X}", command::ec);
            expect(eq(s2, std::string{"0xF7"}));
        };

        // ------------------------------------------------------------
        // unknown command formatting
        // ------------------------------------------------------------

        "unknown command formats as UNKNOWN in name mode"_test = [] mutable {
            auto unknown = static_cast<command>(0x0A);
            auto s       = std::format("{:n}", unknown);
            expect(eq(s, std::string{"UNKNOWN"}));
        };

        "unknown command formats as hex in hex mode"_test = [] mutable {
            auto unknown = static_cast<command>(0x0A);
            auto s1      = std::format("{:x}", unknown);
            expect(eq(s1, std::string{"0x0a"}));
            auto s2 = std::format("{:X}", unknown);
            expect(eq(s2, std::string{"0x0A"}));
        };

        // ------------------------------------------------------------
        // invalid format specifier throws
        // ------------------------------------------------------------

        "invalid command format specifier throws"_test = [] mutable {
            bool threw = false;
            try {
                [[maybe_unused]] auto x = std::format("{:z}", command::iac);
            } catch (const std::format_error&) {
                threw = true;
            }
            expect(eq(threw, true));
        };

        // ------------------------------------------------------------
        // negotiation_direction formatting
        // ------------------------------------------------------------

        "negotiation_direction formats correctly"_test = [] mutable {
            auto s1 = std::format("{}", negotiation_direction::local);
            auto s2 = std::format("{}", negotiation_direction::remote);

            expect(eq(s1, std::string{"local"}));
            expect(eq(s2, std::string{"remote"}));
        };

        "invalid negotiation_direction format throws"_test = [] mutable {
            bool threw = false;
            try {
                [[maybe_unused]] auto x = std::format("{:x}", negotiation_direction::local);
            } catch (const std::format_error&) {
                threw = true;
            }
            expect(eq(threw, true));
        };

        // ============================================================
        // Composability inside larger formatted expressions
        // ============================================================

        "formatter composes inside nested format expressions"_test = [] mutable {
            auto msg = std::format("[{}:{}]", command::iac, negotiation_direction::remote);
            expect(eq(msg, std::string{"[IAC (0xFF):remote]"}));
        };
    };
} //namespace

int main() {}
