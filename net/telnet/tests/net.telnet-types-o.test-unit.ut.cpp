// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:types
// Black-box tests using only exported interface.

import net.telnet;

import std;
import ut;

using namespace net::telnet;
using namespace ut;

// ============================================================
// byte_t
// ============================================================

"byte_t contract"_test = [] mutable
{
    expect(sizeof(byte_t) == 1);
    expect(std::is_unsigned_v<byte_t>);
    expect(std::numeric_limits<byte_t>::digits == 8);
}

// ============================================================
// command enum structural properties
// ============================================================

"command enum structural guarantees"_test = [] mutable
{
    expect(std::is_enum_v<command>);
    expect(std::is_same_v<std::underlying_type_t<command>, byte_t>);
}

"command numeric values match RFC definitions"_test = [] mutable
{
    expect(std::to_underlying(command::eor) == 0xEF);
    expect(std::to_underlying(command::se) == 0xF0);
    expect(std::to_underlying(command::nop) == 0xF1);
    expect(std::to_underlying(command::dm) == 0xF2);
    expect(std::to_underlying(command::brk) == 0xF3);
    expect(std::to_underlying(command::ip) == 0xF4);
    expect(std::to_underlying(command::ao) == 0xF5);
    expect(std::to_underlying(command::ayt) == 0xF6);
    expect(std::to_underlying(command::ec) == 0xF7);
    expect(std::to_underlying(command::el) == 0xF8);
    expect(std::to_underlying(command::ga) == 0xF9);
    expect(std::to_underlying(command::sb) == 0xFA);
    expect(std::to_underlying(command::will_opt) == 0xFB);
    expect(std::to_underlying(command::wont_opt) == 0xFC);
    expect(std::to_underlying(command::do_opt) == 0xFD);
    expect(std::to_underlying(command::dont_opt) == 0xFE);
    expect(std::to_underlying(command::iac) == 0xFF);
}

// ============================================================
// command formatter — default behavior
// ============================================================

"command default formatting (d)"_test = [] mutable
{
    expect(std::format("{}", command::will_opt) == "WILL (0xfb)");
    expect(std::format("{}", command::ip) == "IP (0xf4)");
}

"command name-only formatting (n)"_test = [] mutable
{
    expect(std::format("{:n}", command::do_opt) == "DO");
    expect(std::format("{:n}", command::dont_opt) == "DONT");
}

"command hex-only formatting (x)"_test = [] mutable
{
    expect(std::format("{:x}", command::sb) == "0xfa");
    expect(std::format("{:x}", command::iac) == "0xff");
}

"command formatting inside composite string"_test = [] mutable
{
    auto s = std::format("Received {}", command::ao);
    expect(s == "Received AO (0xf5)");
}

// ============================================================
// command unknown-value behavior
// ============================================================

"unknown command formatting behavior"_test = [] mutable
{
    command unknown = static_cast<command>(0x01);

    expect(std::format("{:n}", unknown) == "UNKNOWN");
    expect(std::format("{:x}", unknown) == "0x01");
    expect(std::format("{}", unknown) == "UNKNOWN (0x01)");
}

// ============================================================
// command formatter error handling
// ============================================================

"invalid command format specifier throws"_test = [] mutable
{
    bool threw = false;
    try {
        std::format("{:q}", command::nop);
    } catch(std::format_error& e) {
        threw = true;
    }
    expect(threw);
}

// ============================================================
// negotiation_direction structural properties
// ============================================================

"negotiation_direction structural guarantees"_test = [] mutable
{
    expect(std::is_enum_v<negotiation_direction>);
    expect(std::is_same_v<std::underlying_type_t<negotiation_direction>, std::uint8_t>);
}

// ============================================================
// negotiation_direction formatting
// ============================================================

"negotiation_direction formatting"_test = [] mutable
{
    expect(std::format("{}", negotiation_direction::local) == "local");
    expect(std::format("{}", negotiation_direction::remote) == "remote");
}

"negotiation_direction invalid specifier throws"_test = [] mutable
{
    bool threw = false;
    try {
        std::format("{:x}", negotiation_direction::local);
    } catch(std::format_error& e) {
        threw = true;
    }
    expect(threw);
}
