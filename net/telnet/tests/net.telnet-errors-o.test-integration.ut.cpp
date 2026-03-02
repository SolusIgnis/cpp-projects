// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:errors
// Validates std::error_code interoperability and condition mapping.

import net.telnet;
import std;
import ut;

using namespace ut;
using namespace net::telnet;

// ============================================================
// default_error_condition mapping correctness
// ============================================================

"protocol-related errors map to std::errc::protocol_error"_test = []
{
    std::error_code ec = error::invalid_command;

    expect(ec.default_error_condition() == std::make_error_condition(std::errc::protocol_error));
}

"option_not_available maps to not_supported"_test = []
{
    std::error_code ec = error::option_not_available;

    expect(ec.default_error_condition() == std::make_error_condition(std::errc::not_supported));
}

"subnegotiation_overflow maps to message_size"_test = []
{
    std::error_code ec = error::subnegotiation_overflow;

    expect(ec.default_error_condition() == std::make_error_condition(std::errc::message_size));
}

"internal_error maps to state_not_recoverable"_test = []
{
    std::error_code ec = error::internal_error;

    expect(ec.default_error_condition() == std::make_error_condition(std::errc::state_not_recoverable));
}

"user_handler_forbidden maps to operation_not_permitted"_test = []
{
    std::error_code ec = error::user_handler_forbidden;

    expect(ec.default_error_condition() == std::make_error_condition(std::errc::operation_not_permitted));
}

// ============================================================
// error_code equivalence semantics
// ============================================================

"error_code equivalence with std::errc"_test = []
{
    std::error_code ec = error::invalid_negotiation;

    expect(ec == std::errc::protocol_error);
    REQUIRE_FALSE(ec == std::errc::not_supported);
}

// ============================================================
// Category isolation
// ============================================================

"error and processing_signal categories are distinct"_test = []
{
    std::error_code e1 = error::protocol_violation;
    std::error_code e2 = processing_signal::end_of_line;

    expect(e1.category() != e2.category());
    expect(e1 != e2);
}

// ============================================================
// Full enum range robustness (no UB)
// ============================================================

"all defined error values produce valid error_code"_test = []
{
    for (int v = 1; v <= static_cast<int>(error::negotiation_queue_error); ++v) {
        auto ec = make_error_code(static_cast<error>(v));
        expect(ec.category() == telnet_error_category::instance());
        REQUIRE_NOTHROW(ec.message());
    }
}

"all defined processing_signal values produce valid error_code"_test = []
{
    for (int v = 1; v <= static_cast<int>(processing_signal::data_mark); ++v) {
        auto ec = make_error_code(static_cast<processing_signal>(v));
        expect(ec.category() == telnet_processing_signal_category::instance());
        REQUIRE_NOTHROW(ec.message());
    }
}
