// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:errors
// Strict black-box tests of error enums and categories.

import boost.ut;
import net.telnet:errors;
import std;

using namespace boost::ut;
using net::telnet::error;
using net::telnet::processing_signal;
using net::telnet::telnet_error_category;
using net::telnet::telnet_processing_signal_category;

suite net_telnet_errors_unit = [] mutable {
//
// Compile-time enum invariants
//

static_assert(std::is_error_code_enum_v<error>,
              "error must be is_error_code_enum");

static_assert(std::is_error_code_enum_v<processing_signal>,
              "processing_signal must be is_error_code_enum");

//
// Runtime tests
//

"telnet_error_category singleton identity"_test = [] mutable {
    auto& a = telnet_error_category::instance();
    auto& b = telnet_error_category::instance();

    expect(eq(&a, &b));
};

"telnet_processing_signal_category singleton identity"_test = [] mutable {
    auto& a = telnet_processing_signal_category::instance();
    auto& b = telnet_processing_signal_category::instance();

    expect(eq(&a, &b));
};

"telnet_error_category name"_test = [] mutable {
    auto& cat = telnet_error_category::instance();
    expect(eq(std::string{cat.name()}, std::string{"telnet"}));
};

"telnet_processing_signal_category name"_test = [] mutable {
    auto& cat = telnet_processing_signal_category::instance();
    expect(eq(std::string{cat.name()},
              std::string{"telnet_processing_signal"}));
};

//
// Error message coverage (EVERY enumerator)
//

"error message coverage"_test = [] mutable {
    auto& cat = telnet_error_category::instance();

    expect(eq(cat.message(static_cast<int>(error::protocol_violation)),
              std::string{"Telnet protocol violation"}));

    expect(eq(cat.message(static_cast<int>(error::internal_error)),
              std::string{"Unexpected internal Telnet error"}));

    expect(eq(cat.message(static_cast<int>(error::invalid_command)),
              std::string{"Unrecognized Telnet command after IAC"}));

    expect(eq(cat.message(static_cast<int>(error::invalid_negotiation)),
              std::string{"Invalid Telnet negotiation command"}));

    expect(eq(cat.message(static_cast<int>(error::option_not_available)),
              std::string{"Telnet option not available"}));

    expect(eq(cat.message(static_cast<int>(error::invalid_subnegotiation)),
              std::string{"Invalid or incomplete Telnet subnegotiation"}));

    expect(eq(cat.message(static_cast<int>(error::subnegotiation_overflow)),
              std::string{"Telnet subnegotiation buffer overflow"}));

    expect(eq(cat.message(static_cast<int>(error::ignored_go_ahead)),
              std::string{"Telnet Go-Ahead ignored due to SUPPRESS_GO_AHEAD"}));

    expect(eq(cat.message(static_cast<int>(error::user_handler_forbidden)),
              std::string{"Attempt to register handler for reserved option"}));

    expect(eq(cat.message(static_cast<int>(error::user_handler_not_found)),
              std::string{"No handler registered for requested option"}));

    expect(eq(cat.message(static_cast<int>(error::negotiation_queue_error)),
              std::string{
                  "Telnet negotiation queue bit can only be set when the "
                  "NegotiationState is WANTYES or WANTNO."}));
};

"unknown error message fallback"_test = [] mutable {
    auto& cat = telnet_error_category::instance();

    constexpr int invalid_value = 255;
    expect(eq(cat.message(invalid_value),
              std::string{"Unknown Telnet error"}));
};

//
// Processing signal message coverage
//

"processing_signal message coverage"_test = [] mutable {
    auto& cat = telnet_processing_signal_category::instance();

    expect(eq(cat.message(static_cast<int>(processing_signal::end_of_line)),
              std::string{"Telnet encountered End-of-Line in the byte stream"}));

    expect(eq(cat.message(static_cast<int>(processing_signal::carriage_return)),
              std::string{
                  "Telnet encountered Carriage-Return sequence in the byte stream requiring special handling"}));

    expect(eq(cat.message(static_cast<int>(processing_signal::end_of_record)),
              std::string{
                  "Telnet encountered \"End-of-Record\" command in the byte stream"}));

    expect(eq(cat.message(static_cast<int>(processing_signal::go_ahead)),
              std::string{
                  "Telnet encountered \"Go-Ahead\" command in the byte stream"}));

    expect(eq(cat.message(static_cast<int>(processing_signal::erase_character)),
              std::string{
                  "Telnet encountered \"Erase Character\" command in the byte stream"}));

    expect(eq(cat.message(static_cast<int>(processing_signal::erase_line)),
              std::string{
                  "Telnet encountered \"Erase Line\" command in the byte stream"}));

    expect(eq(cat.message(static_cast<int>(processing_signal::abort_output)),
              std::string{
                  "Telnet encountered \"Abort Output\" command in the byte stream"}));

    expect(eq(cat.message(static_cast<int>(processing_signal::interrupt_process)),
              std::string{
                  "Telnet encountered \"Interrupt Process\" command in the byte stream"}));

    expect(eq(cat.message(static_cast<int>(processing_signal::telnet_break)),
              std::string{
                  "Telnet encountered \"Break\" command in the byte stream"}));

    expect(eq(cat.message(static_cast<int>(processing_signal::data_mark)),
              std::string{
                  "Telnet encountered \"Data Mark\" command in the byte stream"}));
};

"unknown processing_signal message fallback"_test = [] mutable {
    auto& cat = telnet_processing_signal_category::instance();

    constexpr int invalid_value = 255;
    expect(eq(cat.message(invalid_value),
              std::string{"Unknown Telnet processing signal"}));
};
};

int main() {}
