// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:types
// Black-box tests using only exported interface.

import net.telnet;

import std;
import ut;

using namespace net::telnet;
using namespace ut;

int main()
{
    // ============================================================
    // byte_t
    // ============================================================

    "byte_t contract"_test = [] mutable {
        expect(sizeof(byte_t) == 1_i);
        expect(_b{true} == std::is_unsigned_v<byte_t>);
        expect(std::numeric_limits<byte_t>::digits == 8_i);
    };

    // ============================================================
    // command enum structural properties
    // ============================================================

    "command enum structural guarantees"_test = [] mutable {
        expect(_b{true} == std::is_enum_v<command>);
        expect(_b{true} == std::is_same_v<std::underlying_type_t<command>, byte_t>);
    };

    "command numeric values match RFC definitions"_test = [] mutable {
        expect(std::to_underlying(command::eor) == 0xEF_i);
        expect(std::to_underlying(command::se) == 0xF0_i);
        expect(std::to_underlying(command::nop) == 0xF1_i);
        expect(std::to_underlying(command::dm) == 0xF2_i);
        expect(std::to_underlying(command::brk) == 0xF3_i);
        expect(std::to_underlying(command::ip) == 0xF4_i);
        expect(std::to_underlying(command::ao) == 0xF5_i);
        expect(std::to_underlying(command::ayt) == 0xF6_i);
        expect(std::to_underlying(command::ec) == 0xF7_i);
        expect(std::to_underlying(command::el) == 0xF8_i);
        expect(std::to_underlying(command::ga) == 0xF9_i);
        expect(std::to_underlying(command::sb) == 0xFA_i);
        expect(std::to_underlying(command::will_opt) == 0xFB_i);
        expect(std::to_underlying(command::wont_opt) == 0xFC_i);
        expect(std::to_underlying(command::do_opt) == 0xFD_i);
        expect(std::to_underlying(command::dont_opt) == 0xFE_i);
        expect(std::to_underlying(command::iac) == 0xFF_i);
    };

    // ============================================================
    // command formatter — default behavior
    // ============================================================

    "command default formatting (d)"_test = [] mutable {
        expect(std::format("{}", command::will_opt) == "WILL (0xfb)");
        expect(std::format("{}", command::ip) == "IP (0xf4)");
    };

    "command name-only formatting (n)"_test = [] mutable {
        expect(std::format("{:n}", command::do_opt) == "DO");
        expect(std::format("{:n}", command::dont_opt) == "DONT");
    };

    "command hex-only formatting (x)"_test = [] mutable {
        expect(std::format("{:x}", command::sb) == "0xfa");
        expect(std::format("{:x}", command::iac) == "0xff");
    };

    "command formatting inside composite string"_test = [] mutable {
        auto s = std::format("Received {}", command::ao);
        expect(s == "Received AO (0xf5)");
    };

    // ============================================================
    // command unknown-value behavior
    // ============================================================

    "unknown command formatting behavior"_test = [] mutable {
        command unknown = static_cast<command>(0x01);

        expect(std::format("{:n}", unknown) == "UNKNOWN");
        expect(std::format("{:x}", unknown) == "0x01");
        expect(std::format("{}", unknown) == "UNKNOWN (0x01)");
    };

    // ============================================================
    // command formatter error handling
    // ============================================================

    "invalid command format specifier throws"_test = [] mutable {
        bool threw = false;
        try {
            std::format("{:q}", command::nop);
        } catch (std::format_error& e) {
            threw = true;
        }
        expect(_b{true} == threw);
    };

    // ============================================================
    // negotiation_direction structural properties
    // ============================================================

    "negotiation_direction structural guarantees"_test = [] mutable {
        expect(_b{true} == std::is_enum_v<negotiation_direction>);
        expect(_b{true} == std::is_same_v<std::underlying_type_t<negotiation_direction>, std::uint8_t>);
    };

    // ============================================================
    // negotiation_direction formatting
    // ============================================================

    "negotiation_direction formatting"_test = [] mutable {
        expect(std::format("{}", negotiation_direction::local) == "local");
        expect(std::format("{}", negotiation_direction::remote) == "remote");
    };

    "negotiation_direction invalid specifier throws"_test = [] mutable {
        bool threw = false;
        try {
            std::format("{:x}", negotiation_direction::local);
        } catch (std::format_error& e) {
            threw = true;
        }
        expect(_b{true} == threw);
    };

    return 0;
}
