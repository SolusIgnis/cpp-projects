// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:errors
// Strict black-box tests of error enums and categories.

import net.telnet;
import std;
import ut;

using namespace ut;
using namespace std::literals;
using net::telnet::error;
using net::telnet::processing_signal;
using net::telnet::telnet_error_category;
using net::telnet::telnet_processing_signal_category;

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    // ============================================================
    // Enum structural guarantees
    // ============================================================

    "error enum structural guarantees"_test = [] mutable {
        expect(eq(std::is_enum_v<error>, true));
        expect(eq(sizeof(error), std::size_t{1}));
        expect(eq(std::is_error_code_enum_v<error>, true));
    };

    "processing_signal enum structural guarantees"_test = [] mutable {
        expect(eq(std::is_enum_v<processing_signal>, true));
        expect(eq(sizeof(processing_signal), std::size_t{1}));
        expect(eq(std::is_error_code_enum_v<processing_signal>, true));
    };

    // ============================================================
    // Category identity and singleton behavior
    // ============================================================

    "telnet_error_category singleton identity"_test = [] mutable {
        const auto& ref1 = telnet_error_category::instance();
        const auto& ref2 = telnet_error_category::instance();

        expect(eq(&ref1, &ref2));
    };

    "telnet_processing_signal_category singleton identity"_test = [] mutable {
        const auto& ref1 = telnet_processing_signal_category::instance();
        const auto& ref2 = telnet_processing_signal_category::instance();

        expect(eq(&ref1, &ref2));
    };

    "telnet_error_category name"_test = [] mutable {
        const auto& cat = telnet_error_category::instance();
        expect(eq(std::string{cat.name()}, "telnet"s));
    };

    "telnet_processing_signal_category name"_test = [] mutable {
        const auto& cat = telnet_processing_signal_category::instance();
        expect(eq(std::string{cat.name()}, "telnet_processing_signal"s));
    };

    // ============================================================
    // Message correctness
    // ============================================================

    "error message coverage"_test = [] mutable {
        const auto& cat = telnet_error_category::instance();

        expect(eq(cat.message(static_cast<int>(error::protocol_violation)), "Telnet protocol violation"s));

        expect(eq(cat.message(static_cast<int>(error::internal_error)), "Unexpected internal Telnet error"s));

        expect(eq(cat.message(static_cast<int>(error::invalid_command)), "Unrecognized Telnet command after IAC"s));

        expect(eq(cat.message(static_cast<int>(error::invalid_negotiation)), "Invalid Telnet negotiation command"s));

        expect(eq(cat.message(static_cast<int>(error::option_not_available)), "Telnet option not available"s));

        expect(eq(cat.message(static_cast<int>(error::invalid_subnegotiation)), "Invalid or incomplete Telnet subnegotiation"s));

        expect(eq(cat.message(static_cast<int>(error::subnegotiation_overflow)), "Telnet subnegotiation buffer overflow"s));

        expect(eq(cat.message(static_cast<int>(error::ignored_go_ahead)), "Telnet Go-Ahead ignored due to SUPPRESS_GO_AHEAD"s));

        expect(eq(cat.message(static_cast<int>(error::user_handler_forbidden)), "Attempt to register handler for reserved option"s));

        expect(eq(cat.message(static_cast<int>(error::user_handler_not_found)), "No handler registered for requested option"s));

        expect(
            eq(cat.message(static_cast<int>(error::negotiation_queue_error)),
               "Telnet negotiation queue bit can only be set when the " "NegotiationState is WANTYES or WANTNO."s)
        );
    };

    "processing_signal message coverage"_test = [] mutable {
        const auto& cat = telnet_processing_signal_category::instance();

        expect(eq(cat.message(static_cast<int>(processing_signal::end_of_line)), "Telnet encountered End-of-Line in the byte stream"s));

        expect(
            eq(cat.message(static_cast<int>(processing_signal::carriage_return)),
               "Telnet encountered Carriage-Return sequence in the byte stream requiring special handling"})
        );

        expect(eq(cat.message(static_cast<int>(processing_signal::end_of_record)), "Telnet encountered \"End-of-Record\" command in the byte stream"s));

        expect(eq(cat.message(static_cast<int>(processing_signal::go_ahead)), "Telnet encountered \"Go-Ahead\" command in the byte stream"s));

        expect(eq(cat.message(static_cast<int>(processing_signal::erase_character)), "Telnet encountered \"Erase Character\" command in the byte stream"s));

        expect(eq(cat.message(static_cast<int>(processing_signal::erase_line)), "Telnet encountered \"Erase Line\" command in the byte stream"s));

        expect(eq(cat.message(static_cast<int>(processing_signal::abort_output)), "Telnet encountered \"Abort Output\" command in the byte stream"s));

        expect(
            eq(cat.message(static_cast<int>(processing_signal::interrupt_process)),
               "Telnet encountered \"Interrupt Process\" command in the byte stream"s)
        );

        expect(eq(cat.message(static_cast<int>(processing_signal::telnet_break)), "Telnet encountered \"Break\" command in the byte stream"s));

        expect(eq(cat.message(static_cast<int>(processing_signal::data_mark)), "Telnet encountered \"Data Mark\" command in the byte stream"s));
    };

    // ============================================================
    // Unknown value safety
    // ============================================================

    "unknown error message fallback"_test = [] mutable {
        const auto& cat = telnet_error_category::instance();

        constexpr int invalid_value = 255;
        expect(eq(cat.message(invalid_value), "Unknown Telnet error"s));
    };

    "unknown processing_signal message fallback"_test = [] mutable {
        const auto& cat = telnet_processing_signal_category::instance();

        constexpr int invalid_value = 255;
        expect(eq(cat.message(invalid_value), "Unknown Telnet processing signal"s));
    };
}
