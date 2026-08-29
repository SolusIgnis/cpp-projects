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

namespace {
//NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
suite net_telnet_errors_integration = [] mutable {
    // ============================================================
    // make_error_code correctness
    // ============================================================

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

    // ============================================================
    // default_error_condition mapping correctness
    // ============================================================

    "default_error_condition mapping"_test = [] mutable {
        auto expect_ec_mapping = [](std::error_code code, std::error_condition expected_condition) {
            auto cond = code.default_error_condition();

            expect(eq(cond.value(), expected_condition.value()));
            expect(eq(&cond.category(), &expected_condition.category()));
        };

        expect_ec_mapping(make_error_code(error::protocol_violation), make_error_condition(std::errc::protocol_error));
        expect_ec_mapping(make_error_code(error::option_not_available), make_error_condition(std::errc::not_supported));
        expect_ec_mapping(make_error_code(error::subnegotiation_overflow), make_error_condition(std::errc::message_size));
        expect_ec_mapping(make_error_code(error::internal_error), make_error_condition(std::errc::state_not_recoverable));
        expect_ec_mapping(
            make_error_code(error::user_handler_forbidden), make_error_condition(std::errc::operation_not_permitted)
        );
    };

    // ============================================================
    // error_code equivalence semantics
    // ============================================================

    "error_code equivalence with std::errc"_test = [] mutable {
        std::error_code ec = error::invalid_negotiation;

        expect(eq((ec == std::errc::protocol_error), true));
        expect(eq((ec != std::errc::not_supported), true));
    };

    // ============================================================
    // Category isolation
    // ============================================================

    "error and processing_signal categories are distinct"_test = [] mutable {
        std::error_code e1 = error::protocol_violation;
        std::error_code e2 = processing_signal::end_of_line;

        expect(neq(&e1.category(), &e2.category()));
        expect(neq(e1, e2));
    };
};
} //namespace

int main() {}
