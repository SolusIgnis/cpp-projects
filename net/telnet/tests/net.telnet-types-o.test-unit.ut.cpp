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
        expect(eq(std::to_underlying(command::eor), 0xEF_i));
        expect(eq(std::to_underlying(command::se), 0xF0_i));
        expect(eq(std::to_underlying(command::nop), 0xF1_i));
        expect(eq(std::to_underlying(command::dm), 0xF2_i));
        expect(eq(std::to_underlying(command::brk), 0xF3_i));
        expect(eq(std::to_underlying(command::ip), 0xF4_i));
        expect(eq(std::to_underlying(command::ao), 0xF5_i));
        expect(eq(std::to_underlying(command::ayt), 0xF6_i));
        expect(eq(std::to_underlying(command::ec), 0xF7_i));
        expect(eq(std::to_underlying(command::el), 0xF8_i));
        expect(eq(std::to_underlying(command::ga), 0xF9_i));
        expect(eq(std::to_underlying(command::sb), 0xFA_i));
        expect(eq(std::to_underlying(command::will_opt), 0xFB_i));
        expect(eq(std::to_underlying(command::wont_opt), 0xFC_i));
        expect(eq(std::to_underlying(command::do_opt), 0xFD_i));
        expect(eq(std::to_underlying(command::dont_opt), 0xFE_i));
        expect(eq(std::to_underlying(command::iac), 0xFF_i));
    };

    // ============================================================
    // negotiation_direction structural properties
    // ============================================================

    "negotiation_direction structural guarantees"_test = [] mutable {
        expect(eq(std::is_enum_v<negotiation_direction>));
        expect(eq(sizeof(negotiation_direction), std::size_t{1}));
    };
};

int main() {}
