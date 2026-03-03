// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:types

import net.telnet;
import ut;
import std;

using namespace ut;

suite net_telnet_types_unit = [] mutable {
    using net::telnet::byte_t;
    using net::telnet::command;
    using net::telnet::negotiation_direction;

    // ------------------------------------------------------------
    // byte_t invariants
    // ------------------------------------------------------------

    "byte_t contract"_test = [] mutable {
        expect(eq(sizeof(byte_t), std::size_t{1}));
        expect(eq(std::is_unsigned_v<byte_t>, true));
    };

    // ============================================================
    // command enum structural properties
    // ============================================================

    "command enum structural guarantees"_test = [] mutable {
        expect(eq(std::is_enum_v<command>, true));
        expect(eq(std::is_same_v<std::underlying_type_t<command>, byte_t>, true));
    };

    // ------------------------------------------------------------
    // command underlying values (RFC invariants)
    // ------------------------------------------------------------

    "command numeric values match RFC definitions"_test = [] mutable {
        expect(eq(std::to_underlying(command::eor), static_cast<std::underlying_type_t<command>>(0xEF)));
        expect(eq(std::to_underlying(command::se), static_cast<std::underlying_type_t<command>>(0xF0)));
        expect(eq(std::to_underlying(command::nop), static_cast<std::underlying_type_t<command>>(0xF1)));
        expect(eq(std::to_underlying(command::dm), static_cast<std::underlying_type_t<command>>(0xF2)));
        expect(eq(std::to_underlying(command::brk), static_cast<std::underlying_type_t<command>>(0xF3)));
        expect(eq(std::to_underlying(command::ip), static_cast<std::underlying_type_t<command>>(0xF4)));
        expect(eq(std::to_underlying(command::ao), static_cast<std::underlying_type_t<command>>(0xF5)));
        expect(eq(std::to_underlying(command::ayt), static_cast<std::underlying_type_t<command>>(0xF6)));
        expect(eq(std::to_underlying(command::ec), static_cast<std::underlying_type_t<command>>(0xF7)));
        expect(eq(std::to_underlying(command::el), static_cast<std::underlying_type_t<command>>(0xF8)));
        expect(eq(std::to_underlying(command::ga), static_cast<std::underlying_type_t<command>>(0xF9)));
        expect(eq(std::to_underlying(command::sb), static_cast<std::underlying_type_t<command>>(0xFA)));
        expect(eq(std::to_underlying(command::will_opt), static_cast<std::underlying_type_t<command>>(0xFB)));
        expect(eq(std::to_underlying(command::wont_opt), static_cast<std::underlying_type_t<command>>(0xFC)));
        expect(eq(std::to_underlying(command::do_opt), static_cast<std::underlying_type_t<command>>(0xFD)));
        expect(eq(std::to_underlying(command::dont_opt), static_cast<std::underlying_type_t<command>>(0xFE)));
        expect(eq(std::to_underlying(command::iac), static_cast<std::underlying_type_t<command>>(0xFF)));
    };

    // ============================================================
    // negotiation_direction structural properties
    // ============================================================

    "negotiation_direction structural guarantees"_test = [] mutable {
        expect(eq(std::is_enum_v<negotiation_direction>, true));
        expect(eq(sizeof(negotiation_direction), std::size_t{1}));
    };
};

int main() {}
