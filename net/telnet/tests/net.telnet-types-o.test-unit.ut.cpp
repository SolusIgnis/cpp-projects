// SPDX-License-Identifier: Apache-2.0
// net.telnet-types-o.test-unit.ut.cpp
// Black-box unit tests for net.telnet:types

import net.telnet;
import ut;
import std;

using namespace ut;

suite net_telnet_types_byte_tests = [] mutable {
    using net::telnet::byte_t;

    "byte_t is exactly one byte"_test = [] mutable { expect(sizeof(byte_t) == 1_u); };

    "byte_t behaves as unsigned 8-bit storage"_test = [] mutable {
        byte_t b = static_cast<byte_t>(255);
        expect(static_cast<unsigned>(b) == 255_u);
    };
};

suite net_telnet_types_command_value_tests = [] mutable {
    using net::telnet::command;

    "IAC value matches RFC"_test = [] mutable { expect(std::to_underlying(command::iac) == 0xFF_u); };

    "WILL value matches RFC"_test = [] mutable { expect(std::to_underlying(command::will_opt) == 0xFB_u); };

    "WONT value matches RFC"_test = [] mutable { expect(std::to_underlying(command::wont_opt) == 0xFC_u); };

    "DO value matches RFC"_test = [] mutable { expect(std::to_underlying(command::do_opt) == 0xFD_u); };

    "DONT value matches RFC"_test = [] mutable { expect(std::to_underlying(command::dont_opt) == 0xFE_u); };
};

suite net_telnet_types_command_format_tests = [] mutable {
    using net::telnet::command;

    "default format prints name and hex"_test = [] mutable {
        auto s = std::format("{}", command::will_opt);
        expect(s == "WILL (0xfb)");
    };

    "name-only format"_test = [] mutable {
        auto s = std::format("{:n}", command::do_opt);
        expect(s == "DO");
    };

    "hex-only format"_test = [] mutable {
        auto s = std::format("{:x}", command::iac);
        expect(s == "0xff");
    };

    "unknown command name-only prints UNKNOWN"_test = [] mutable {
        auto unknown = static_cast<command>(0x00);
        auto s       = std::format("{:n}", unknown);
        expect(s == "UNKNOWN");
    };

    "unknown command hex-only prints raw hex"_test = [] mutable {
        auto unknown = static_cast<command>(0x01);
        auto s       = std::format("{:x}", unknown);
        expect(s == "0x01");
    };

    "invalid format specifier throws"_test = [] mutable {
        expect(throws<std::format_error>([] { std::format("{:z}", command::iac); }));
    };
};

suite net_telnet_types_negotiation_direction_tests = [] mutable {
    using net::telnet::negotiation_direction;

    "local formats correctly"_test = [] mutable {
        auto s = std::format("{}", negotiation_direction::local);
        expect(s == "local");
    };

    "remote formats correctly"_test = [] mutable {
        auto s = std::format("{}", negotiation_direction::remote);
        expect(s == "remote");
    };

    "invalid specifier throws"_test = [] {
        expect(throws<std::format_error>([] { std::format("{:x}", negotiation_direction::local); }));
    };
};

int main()
{
    return ut::cfg<>.run();
}
