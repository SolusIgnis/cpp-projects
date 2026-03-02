// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:types
// Black-box tests using only exported interface.

import net.telnet;

//import std;

#include <catch2/catch_test_macros.hpp>

#include <format>
#include <type_traits>

using namespace net::telnet;

// ============================================================
// byte_t
// ============================================================

TEST_CASE("byte_t contract", "[net.telnet][types][byte_t]")
{
    STATIC_REQUIRE(sizeof(byte_t) == 1);
    STATIC_REQUIRE(std::is_unsigned_v<byte_t>);
    STATIC_REQUIRE(std::numeric_limits<byte_t>::digits == 8);
}

// ============================================================
// command enum structural properties
// ============================================================

TEST_CASE("command enum structural guarantees", "[net.telnet][types][command]")
{
    STATIC_REQUIRE(std::is_enum_v<command>);
    STATIC_REQUIRE(std::is_same_v<std::underlying_type_t<command>, byte_t>);
}

TEST_CASE("command numeric values match RFC definitions", "[net.telnet][types][command]")
{
    REQUIRE(std::to_underlying(command::eor) == 0xEF);
    REQUIRE(std::to_underlying(command::se) == 0xF0);
    REQUIRE(std::to_underlying(command::nop) == 0xF1);
    REQUIRE(std::to_underlying(command::dm) == 0xF2);
    REQUIRE(std::to_underlying(command::brk) == 0xF3);
    REQUIRE(std::to_underlying(command::ip) == 0xF4);
    REQUIRE(std::to_underlying(command::ao) == 0xF5);
    REQUIRE(std::to_underlying(command::ayt) == 0xF6);
    REQUIRE(std::to_underlying(command::ec) == 0xF7);
    REQUIRE(std::to_underlying(command::el) == 0xF8);
    REQUIRE(std::to_underlying(command::ga) == 0xF9);
    REQUIRE(std::to_underlying(command::sb) == 0xFA);
    REQUIRE(std::to_underlying(command::will_opt) == 0xFB);
    REQUIRE(std::to_underlying(command::wont_opt) == 0xFC);
    REQUIRE(std::to_underlying(command::do_opt) == 0xFD);
    REQUIRE(std::to_underlying(command::dont_opt) == 0xFE);
    REQUIRE(std::to_underlying(command::iac) == 0xFF);
}

// ============================================================
// command formatter — default behavior
// ============================================================

TEST_CASE("command default formatting (d)", "[net.telnet][types][format][command]")
{
    REQUIRE(std::format("{}", command::will_opt) == "WILL (0xfb)");
    REQUIRE(std::format("{}", command::ip) == "IP (0xf4)");
}

TEST_CASE("command name-only formatting (n)", "[net.telnet][types][format][command]")
{
    REQUIRE(std::format("{:n}", command::do_opt) == "DO");
    REQUIRE(std::format("{:n}", command::dont_opt) == "DONT");
}

TEST_CASE("command hex-only formatting (x)", "[net.telnet][types][format][command]")
{
    REQUIRE(std::format("{:x}", command::sb) == "0xfa");
    REQUIRE(std::format("{:x}", command::iac) == "0xff");
}

TEST_CASE("command formatting inside composite string", "[net.telnet][types][format][command]")
{
    auto s = std::format("Received {}", command::ao);
    REQUIRE(s == "Received AO (0xf5)");
}

// ============================================================
// command unknown-value behavior
// ============================================================

TEST_CASE("unknown command formatting behavior", "[net.telnet][types][format][command][unknown]")
{
    command unknown = static_cast<command>(0x01);

    REQUIRE(std::format("{:n}", unknown) == "UNKNOWN");
    REQUIRE(std::format("{:x}", unknown) == "0x01");
    REQUIRE(std::format("{}", unknown) == "UNKNOWN (0x01)");
}

// ============================================================
// command formatter error handling
// ============================================================

TEST_CASE("invalid command format specifier throws", "[net.telnet][types][format][command][error]")
{
    REQUIRE_THROWS_AS(std::format("{:q}", command::nop), std::format_error);
}

// ============================================================
// negotiation_direction structural properties
// ============================================================

TEST_CASE("negotiation_direction structural guarantees", "[net.telnet][types][negotiation_direction]")
{
    STATIC_REQUIRE(std::is_enum_v<negotiation_direction>);
    STATIC_REQUIRE(std::is_same_v<std::underlying_type_t<negotiation_direction>, std::uint8_t>);
}

// ============================================================
// negotiation_direction formatting
// ============================================================

TEST_CASE("negotiation_direction formatting", "[net.telnet][types][format][negotiation_direction]")
{
    REQUIRE(std::format("{}", negotiation_direction::local) == "local");
    REQUIRE(std::format("{}", negotiation_direction::remote) == "remote");
}

TEST_CASE("negotiation_direction invalid specifier throws", "[net.telnet][types][format][negotiation_direction][error]")
{
    REQUIRE_THROWS_AS(std::format("{:x}", negotiation_direction::local), std::format_error);
}
