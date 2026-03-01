// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:errors
// Strict black-box tests of error enums and categories.

import net.telnet;
//import std;

#include <catch2/catch_test_macros.hpp>

#include <string_view>
#include <system_error>
#include <type_traits>

#include <cstdint>

using namespace net::telnet;

// ============================================================
// Enum structural guarantees
// ============================================================

TEST_CASE("error enum structural guarantees", "[net.telnet][errors][error]")
{
    STATIC_REQUIRE(std::is_enum_v<error>);
    STATIC_REQUIRE(std::is_same_v<std::underlying_type_t<error>, std::uint8_t>);
    STATIC_REQUIRE(std::is_error_code_enum_v<error>);
}

TEST_CASE("processing_signal enum structural guarantees", "[net.telnet][errors][processing_signal]")
{
    STATIC_REQUIRE(std::is_enum_v<processing_signal>);
    STATIC_REQUIRE(std::is_same_v<std::underlying_type_t<processing_signal>, std::uint8_t>);
    STATIC_REQUIRE(std::is_error_code_enum_v<processing_signal>);
}

// ============================================================
// Category identity and singleton behavior
// ============================================================

TEST_CASE("telnet_error_category is singleton", "[net.telnet][errors][category]")
{
    auto& c1 = telnet_error_category::instance();
    auto& c2 = telnet_error_category::instance();

    REQUIRE(&c1 == &c2);
    REQUIRE(std::string_view(c1.name()) == "telnet");
}

TEST_CASE("telnet_processing_signal_category is singleton", "[net.telnet][errors][category]")
{
    auto& c1 = telnet_processing_signal_category::instance();
    auto& c2 = telnet_processing_signal_category::instance();

    REQUIRE(&c1 == &c2);
    REQUIRE(std::string_view(c1.name()) == "telnet_processing_signal");
}

// ============================================================
// make_error_code correctness
// ============================================================

TEST_CASE("make_error_code(error) produces correct category", "[net.telnet][errors][make_error_code]")
{
    std::error_code ec = error::protocol_violation;

    REQUIRE(ec.category() == telnet_error_category::instance());
    REQUIRE(ec.value() == static_cast<int>(error::protocol_violation));
}

TEST_CASE("make_error_code(processing_signal) produces correct category", "[net.telnet][errors][make_error_code]")
{
    std::error_code ec = processing_signal::end_of_line;

    REQUIRE(ec.category() == telnet_processing_signal_category::instance());
    REQUIRE(ec.value() == static_cast<int>(processing_signal::end_of_line));
}

// ============================================================
// Message correctness
// ============================================================

TEST_CASE("error category returns correct messages", "[net.telnet][errors][message]")
{
    auto& cat = telnet_error_category::instance();

    REQUIRE(cat.message(static_cast<int>(error::protocol_violation)) == "Telnet protocol violation");

    REQUIRE(cat.message(static_cast<int>(error::invalid_command)) == "Unrecognized Telnet command after IAC");
}

TEST_CASE("processing_signal category returns correct messages", "[net.telnet][errors][message]")
{
    auto& cat = telnet_processing_signal_category::instance();

    REQUIRE(
        cat.message(static_cast<int>(processing_signal::go_ahead))
        == "Telnet encountered \"Go-Ahead\" command in the byte stream"
    );
}

// ============================================================
// Unknown value safety
// ============================================================

TEST_CASE("unknown error value returns fallback message", "[net.telnet][errors][unknown]")
{
    auto& cat = telnet_error_category::instance();

    REQUIRE(cat.message(255) == "Unknown Telnet error");
}

TEST_CASE("unknown processing_signal value returns fallback message", "[net.telnet][errors][unknown]")
{
    auto& cat = telnet_processing_signal_category::instance();

    REQUIRE(cat.message(255) == "Unknown Telnet processing signal");
}
