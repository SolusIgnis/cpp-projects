// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:types

import net.telnet;
import ut;
import std;

using namespace ut;

namespace {
//NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
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
        expect(eq(std::to_underlying(command::eor), byte_t{0xEF}));
        expect(eq(std::to_underlying(command::se), byte_t{0xF0}));
        expect(eq(std::to_underlying(command::nop), byte_t{0xF1}));
        expect(eq(std::to_underlying(command::dm), byte_t{0xF2}));
        expect(eq(std::to_underlying(command::brk), byte_t{0xF3}));
        expect(eq(std::to_underlying(command::ip), byte_t{0xF4}));
        expect(eq(std::to_underlying(command::ao), byte_t{0xF5}));
        expect(eq(std::to_underlying(command::ayt), byte_t{0xF6}));
        expect(eq(std::to_underlying(command::ec), byte_t{0xF7}));
        expect(eq(std::to_underlying(command::el), byte_t{0xF8}));
        expect(eq(std::to_underlying(command::ga), byte_t{0xF9}));
        expect(eq(std::to_underlying(command::sb), byte_t{0xFA}));
        expect(eq(std::to_underlying(command::will_opt), byte_t{0xFB}));
        expect(eq(std::to_underlying(command::wont_opt), byte_t{0xFC}));
        expect(eq(std::to_underlying(command::do_opt), byte_t{0xFD}));
        expect(eq(std::to_underlying(command::dont_opt), byte_t{0xFE}));
        expect(eq(std::to_underlying(command::iac), byte_t{0xFF}));
    };

    // ============================================================
    // negotiation_direction structural properties
    // ============================================================

    "negotiation_direction structural guarantees"_test = [] mutable {
        expect(eq(std::is_enum_v<negotiation_direction>, true));
        expect(eq(sizeof(negotiation_direction), std::size_t{1}));
    };
};
} //namespace

int main() {}
