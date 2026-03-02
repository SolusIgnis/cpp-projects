// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:errors
// Strict black-box tests of error enums and categories.

import net.telnet;
import std;
import ut;

using namespace net::telnet;
using namespace ut;

// ============================================================
// Enum structural guarantees
// ============================================================

"error enum structural guarantees"_test = [] mutable
{
    expect(std::is_enum_v<error>);
    expect(std::is_same_v<std::underlying_type_t<error>, std::uint8_t>);
    expect(std::is_error_code_enum_v<error>);
}

"processing_signal enum structural guarantees"_test = [] mutable
{
    expect(std::is_enum_v<processing_signal>);
    expect(std::is_same_v<std::underlying_type_t<processing_signal>, std::uint8_t>);
    expect(std::is_error_code_enum_v<processing_signal>);
}

// ============================================================
// Category identity and singleton behavior
// ============================================================

"telnet_error_category is singleton"_test = [] mutable
{
    auto& c1 = telnet_error_category::instance();
    auto& c2 = telnet_error_category::instance();

    expect(&c1 == &c2);
    expect(std::string_view(c1.name()) == "telnet");
}

"telnet_processing_signal_category is singleton"_test = [] mutable
{
    auto& c1 = telnet_processing_signal_category::instance();
    auto& c2 = telnet_processing_signal_category::instance();

    expect(&c1 == &c2);
    expect(std::string_view(c1.name()) == "telnet_processing_signal");
}

// ============================================================
// make_error_code correctness
// ============================================================

"make_error_code(error) produces correct category"_test = [] mutable
{
    std::error_code ec = error::protocol_violation;

    expect(ec.category() == telnet_error_category::instance());
    expect(ec.value() == static_cast<int>(error::protocol_violation));
}

"make_error_code(processing_signal) produces correct category"_test = [] mutable
{
    std::error_code ec = processing_signal::end_of_line;

    expect(ec.category() == telnet_processing_signal_category::instance());
    expect(ec.value() == static_cast<int>(processing_signal::end_of_line));
}

// ============================================================
// Message correctness
// ============================================================

"error category returns correct messages"_test = [] mutable
{
    auto& cat = telnet_error_category::instance();

    expect(cat.message(static_cast<int>(error::protocol_violation)) == "Telnet protocol violation");

    expect(cat.message(static_cast<int>(error::invalid_command)) == "Unrecognized Telnet command after IAC");
}

"processing_signal category returns correct messages"_test = [] mutable
{
    auto& cat = telnet_processing_signal_category::instance();

    expect(
        cat.message(static_cast<int>(processing_signal::go_ahead))
        == "Telnet encountered \"Go-Ahead\" command in the byte stream"
    );
}

// ============================================================
// Unknown value safety
// ============================================================

"unknown error value returns fallback message"_test = [] mutable
{
    auto& cat = telnet_error_category::instance();

    expect(cat.message(255) == "Unknown Telnet error");
}

"unknown processing_signal value returns fallback message"_test = [] mutable
{
    auto& cat = telnet_processing_signal_category::instance();

    expect(cat.message(255) == "Unknown Telnet processing signal");
}
