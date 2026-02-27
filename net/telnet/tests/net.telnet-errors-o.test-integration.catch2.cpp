// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:errors
// Validates std::error_code interoperability and condition mapping.

import net.telnet;
import std;

#include <catch2/catch_test_macros.hpp>

using namespace net::telnet;

// ============================================================
// default_error_condition mapping correctness
// ============================================================

TEST_CASE("protocol-related errors map to std::errc::protocol_error",
          "[net.telnet][errors][integration]")
{
    std::error_code ec = error::invalid_command;

    REQUIRE(ec.default_error_condition() ==
            std::make_error_condition(std::errc::protocol_error));
}

TEST_CASE("option_not_available maps to not_supported",
          "[net.telnet][errors][integration]")
{
    std::error_code ec = error::option_not_available;

    REQUIRE(ec.default_error_condition() ==
            std::make_error_condition(std::errc::not_supported));
}

TEST_CASE("subnegotiation_overflow maps to message_size",
          "[net.telnet][errors][integration]")
{
    std::error_code ec = error::subnegotiation_overflow;

    REQUIRE(ec.default_error_condition() ==
            std::make_error_condition(std::errc::message_size));
}

TEST_CASE("internal_error maps to state_not_recoverable",
          "[net.telnet][errors][integration]")
{
    std::error_code ec = error::internal_error;

    REQUIRE(ec.default_error_condition() ==
            std::make_error_condition(std::errc::state_not_recoverable));
}

TEST_CASE("user_handler_forbidden maps to operation_not_permitted",
          "[net.telnet][errors][integration]")
{
    std::error_code ec = error::user_handler_forbidden;

    REQUIRE(ec.default_error_condition() ==
            std::make_error_condition(std::errc::operation_not_permitted));
}

// ============================================================
// error_code equivalence semantics
// ============================================================

TEST_CASE("error_code equivalence with std::errc",
          "[net.telnet][errors][integration]")
{
    std::error_code ec = error::invalid_negotiation;

    REQUIRE(ec == std::errc::protocol_error);
    REQUIRE_FALSE(ec == std::errc::not_supported);
}

// ============================================================
// Category isolation
// ============================================================

TEST_CASE("error and processing_signal categories are distinct",
          "[net.telnet][errors][integration]")
{
    std::error_code e1 = error::protocol_violation;
    std::error_code e2 = processing_signal::end_of_line;

    REQUIRE(e1.category() != e2.category());
    REQUIRE(e1 != e2);
}

// ============================================================
// Full enum range robustness (no UB)
// ============================================================

TEST_CASE("all defined error values produce valid error_code",
          "[net.telnet][errors][robustness]")
{
    for (int v = 1; v <= static_cast<int>(error::negotiation_queue_error); ++v)
    {
        auto ec = make_error_code(static_cast<error>(v));
        REQUIRE(ec.category() == telnet_error_category::instance());
        REQUIRE_NOTHROW(ec.message());
    }
}

TEST_CASE("all defined processing_signal values produce valid error_code",
          "[net.telnet][errors][robustness]")
{
    for (int v = 1; v <= static_cast<int>(processing_signal::data_mark); ++v)
    {
        auto ec = make_error_code(static_cast<processing_signal>(v));
        REQUIRE(ec.category() == telnet_processing_signal_category::instance());
        REQUIRE_NOTHROW(ec.message());
    }
}
