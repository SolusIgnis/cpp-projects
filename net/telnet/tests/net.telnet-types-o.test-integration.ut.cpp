// SPDX-License-Identifier: Apache-2.0
// Black-box integration tests for net.telnet:types

import net.telnet;
import ut;
import std;

using namespace ut;

suite net_telnet_types_format_composition_tests = [] mutable {
    using net::telnet::command;
    using net::telnet::negotiation_direction;

    "command integrates with surrounding format text"_test = [] mutable {
        auto s = std::format("Received {}", command::nop);
        expect(eq(s, std::string{"Received NOP (0xf1)"}));
    };

    "multiple commands format sequentially"_test = [] mutable {
        auto s = std::format("{} {}", command::do_opt, command::dont_opt);
        expect(eq(s, std::string{"DO (0xfd) DONT (0xfe)"}));
    };

    "command name-only format in composite string"_test = [] mutable {
        auto s = std::format("Cmd={:n}", command::brk);
        expect(eq(s, std::string{"Cmd=BRK"}));
    };

    "negotiation_direction integrates in formatted output"_test = [] mutable {
        auto s = std::format("Direction: {}", negotiation_direction::remote);
        expect(eq(s, std::string{"Direction: remote"}));
    };
};

suite net_telnet_types_format_stability_tests = [] mutable {
    using net::telnet::command;

    "default format always contains 0x prefix"_test = [] mutable {
        auto s = std::format("{}", command::ga);
        expect(ne(s.find("0x"), std::string::npos));
    };

    "hex-only format always starts with 0x"_test = [] mutable {
        auto s = std::format("{:x}", command::ec);
        expect(eq(s.rfind("0x", 0), std::string::size_type{0}));
    };

    "name-only format never contains 0x"_test = [] mutable {
        auto s = std::format("{:n}", command::ayt);
        expect(eq(s.find("0x"), std::string::npos));
    };
};

int main() {}
