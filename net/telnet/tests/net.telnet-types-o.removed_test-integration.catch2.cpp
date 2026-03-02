// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:types
// Verifies interoperability with std library and full value ranges.

import net.telnet;

#include <catch2/catch_test_macros.hpp>
#include <format>
#include <string>
#include <array>

using namespace net::telnet;

// ============================================================
// All RFC command range formatting must not throw
// ============================================================

TEST_CASE("all RFC command-range values format without throwing", "[net.telnet][types][integration][command]")
{
    for (int value = 0xEF; value <= 0xFF; ++value) {
        auto cmd = static_cast<command>(static_cast<byte_t>(value));

        try {
            REQUIRE_NOTHROW(std::format("{}", cmd));
            REQUIRE_NOTHROW(std::format("{:n}", cmd));
            REQUIRE_NOTHROW(std::format("{:x}", cmd));
        } catch(...) {
            expect(false) << "std::format throws for command: " << cmd;
        }
    }
}

// ============================================================
// Full 0-255 robustness check (no UB, no crashes)
// ============================================================

TEST_CASE("all 256 possible command byte values are safely formattable", "[net.telnet][types][integration][robustness]")
{
    for (int value = 0; value <= 0xFF; ++value) {
        auto cmd = static_cast<command>(static_cast<byte_t>(value));
        REQUIRE_NOTHROW(std::format("{}", cmd));
    }
}

// ============================================================
// Hex formatting preserves underlying value width and lowercase
// ============================================================

TEST_CASE("hex formatting is zero-padded lowercase", "[net.telnet][types][integration][format]")
{
    auto formatted = std::format("{:x}", command::eor);

    REQUIRE(formatted.size() == 4); // "0x??"
    REQUIRE(formatted.starts_with("0x"));
}

// ============================================================
// Composability inside larger formatted expressions
// ============================================================

TEST_CASE("formatter composes inside nested format expressions", "[net.telnet][types][integration][composition]")
{
    auto msg = std::format("[{}:{}]", command::iac, negotiation_direction::remote);

    REQUIRE(msg == "[IAC (0xff):remote]");
}

// ============================================================
// Round-trip consistency check
// ============================================================

TEST_CASE("hex format matches underlying numeric value", "[net.telnet][types][integration][consistency]")
{
    auto cmd = command::sb;
    auto hex = std::format("{:x}", cmd);

    REQUIRE(hex == "0xfa");
}
