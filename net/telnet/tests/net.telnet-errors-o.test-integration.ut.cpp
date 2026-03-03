// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:errors
// Validates std::error_code interoperability and condition mapping.

import net.telnet;
import std;
import ut;

using namespace ut;
using net::telnet::error;
using net::telnet::processing_signal;
using net::telnet::make_error_code;

suite net_telnet_errors_integration = [] mutable {
    "make_error_code(error) integration"_test = [] mutable {
        auto ec = make_error_code(error::invalid_command);

        expect(eq(ec.value(), static_cast<int>(error::invalid_command)));

        expect(eq(std::string{ec.category().name()}, std::string{"telnet"}));
    };

    "make_error_code(processing_signal) integration"_test = [] mutable {
        auto ec = make_error_code(processing_signal::go_ahead);

        expect(eq(ec.value(), static_cast<int>(processing_signal::go_ahead)));

        expect(eq(std::string{ec.category().name()}, std::string{"telnet_processing_signal"}));
    };

    "default_error_condition mapping"_test = [] mutable {
        {
            auto ec       = make_error_code(error::protocol_violation);
            auto cond     = ec.default_error_condition();
            auto expected = std::make_error_condition(std::errc::protocol_error);

            expect(eq(cond.value(), expected.value()));
            expect(eq(&cond.category(), &expected.category()));
        }

        {
            auto ec       = make_error_code(error::option_not_available);
            auto cond     = ec.default_error_condition();
            auto expected = std::make_error_condition(std::errc::not_supported);

            expect(eq(cond.value(), expected.value()));
            expect(eq(&cond.category(), &expected.category()));
        }

        {
            auto ec       = make_error_code(error::subnegotiation_overflow);
            auto cond     = ec.default_error_condition();
            auto expected = std::make_error_condition(std::errc::message_size);

            expect(eq(cond.value(), expected.value()));
            expect(eq(&cond.category(), &expected.category()));
        }

        {
            auto ec       = make_error_code(error::internal_error);
            auto cond     = ec.default_error_condition();
            auto expected = std::make_error_condition(std::errc::state_not_recoverable);

            expect(eq(cond.value(), expected.value()));
            expect(eq(&cond.category(), &expected.category()));
        }

        {
            auto ec       = make_error_code(error::user_handler_forbidden);
            auto cond     = ec.default_error_condition();
            auto expected = std::make_error_condition(std::errc::operation_not_permitted);

            expect(eq(cond.value(), expected.value()));
            expect(eq(&cond.category(), &expected.category()));
        }
    };
};

int main() {}
