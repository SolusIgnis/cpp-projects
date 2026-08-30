// SPDX-License-Identifier: Apache-2.0
// Unit tests for net.telnet:errors
// Strict black-box tests of error enums and categories.

import net.telnet;
import std;
import ut;

using namespace ut;
using net::telnet::error;
using net::telnet::processing_signal;
using net::telnet::telnet_error_category;
using net::telnet::telnet_processing_signal_category;

namespace {
    //NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
    suite net_telnet_errors_unit = [] mutable {
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
            expect(eq(std::string{cat.name()}, std::string{"telnet"}));
        };

        "telnet_processing_signal_category name"_test = [] mutable {
            const auto& cat = telnet_processing_signal_category::instance();
            expect(eq(std::string{cat.name()}, std::string{"telnet_processing_signal"}));
        };

        // ============================================================
        // Message correctness
        // ============================================================

        "error message coverage"_test = [] mutable {
            const auto& cat = telnet_error_category::instance();

            expect(eq(cat.message(static_cast<int>(error::protocol_violation)), std::string{"Telnet protocol violation"}));

            expect(eq(cat.message(static_cast<int>(error::internal_error)), std::string{"Unexpected internal Telnet error"}));

            expect(
                eq(cat.message(static_cast<int>(error::invalid_command)), std::string{"Unrecognized Telnet command after IAC"})
            );

            expect(
                eq(cat.message(static_cast<int>(error::invalid_negotiation)), std::string{"Invalid Telnet negotiation command"})
            );

            expect(eq(cat.message(static_cast<int>(error::option_not_available)), std::string{"Telnet option not available"}));

            expect(
                eq(cat.message(static_cast<int>(error::invalid_subnegotiation)),
                   std::string{"Invalid or incomplete Telnet subnegotiation"})
            );

            expect(
                eq(cat.message(static_cast<int>(error::subnegotiation_overflow)),
                   std::string{"Telnet subnegotiation buffer overflow"})
            );

            expect(
                eq(cat.message(static_cast<int>(error::ignored_go_ahead)),
                   std::string{"Telnet Go-Ahead ignored due to SUPPRESS_GO_AHEAD"})
            );

            expect(
                eq(cat.message(static_cast<int>(error::user_handler_forbidden)),
                   std::string{"Attempt to register handler for reserved option"})
            );

            expect(
                eq(cat.message(static_cast<int>(error::user_handler_not_found)),
                   std::string{"No handler registered for requested option"})
            );

            expect(
                eq(cat.message(static_cast<int>(error::negotiation_queue_error)),
                   std::string{
                       "Telnet negotiation queue bit can only be set when the " "NegotiationState is WANTYES or WANTNO."
                   })
            );
        };

        "processing_signal message coverage"_test = [] mutable {
            const auto& cat = telnet_processing_signal_category::instance();

            expect(
                eq(cat.message(static_cast<int>(processing_signal::end_of_line)),
                   std::string{"Telnet encountered End-of-Line in the byte stream"})
            );

            expect(
                eq(cat.message(static_cast<int>(processing_signal::carriage_return)),
                   std::string{"Telnet encountered Carriage-Return sequence in the byte stream requiring special handling"})
            );

            expect(
                eq(cat.message(static_cast<int>(processing_signal::end_of_record)),
                   std::string{"Telnet encountered \"End-of-Record\" command in the byte stream"})
            );

            expect(
                eq(cat.message(static_cast<int>(processing_signal::go_ahead)),
                   std::string{"Telnet encountered \"Go-Ahead\" command in the byte stream"})
            );

            expect(
                eq(cat.message(static_cast<int>(processing_signal::erase_character)),
                   std::string{"Telnet encountered \"Erase Character\" command in the byte stream"})
            );

            expect(
                eq(cat.message(static_cast<int>(processing_signal::erase_line)),
                   std::string{"Telnet encountered \"Erase Line\" command in the byte stream"})
            );

            expect(
                eq(cat.message(static_cast<int>(processing_signal::abort_output)),
                   std::string{"Telnet encountered \"Abort Output\" command in the byte stream"})
            );

            expect(
                eq(cat.message(static_cast<int>(processing_signal::interrupt_process)),
                   std::string{"Telnet encountered \"Interrupt Process\" command in the byte stream"})
            );

            expect(
                eq(cat.message(static_cast<int>(processing_signal::telnet_break)),
                   std::string{"Telnet encountered \"Break\" command in the byte stream"})
            );

            expect(
                eq(cat.message(static_cast<int>(processing_signal::data_mark)),
                   std::string{"Telnet encountered \"Data Mark\" command in the byte stream"})
            );
        };

        // ============================================================
        // Unknown value safety
        // ============================================================

        "unknown error message fallback"_test = [] mutable {
            const auto& cat = telnet_error_category::instance();

            constexpr int invalid_value = 255;
            expect(eq(cat.message(invalid_value), std::string{"Unknown Telnet error"}));
        };

        "unknown processing_signal message fallback"_test = [] mutable {
            const auto& cat = telnet_processing_signal_category::instance();

            constexpr int invalid_value = 255;
            expect(eq(cat.message(invalid_value), std::string{"Unknown Telnet processing signal"}));
        };
    };
} //namespace

int main() {}
