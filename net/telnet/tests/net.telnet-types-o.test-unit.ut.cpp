// SPDX-License-Identifier: Apache-2.0
// Black-box unit tests for net.telnet:types

import net.telnet;
import ut;
import std;

using namespace ut;

suite net_telnet_unit = [] mutable {
    "byte_t is exactly 1 byte"_test = [] mutable {
        using net::telnet::byte_t;
        expect(eq(sizeof(byte_t), std::size_t{1}));
    };

    "command formatting is stable"_test = [] mutable {
        using net::telnet::command;

        auto s = std::format("{}", command::will_opt);
        expect(eq(s, std::string{"WILL (0xfb)"}));
    };

    "invalid format specifier throws format_error"_test = [] mutable {
        using net::telnet::command;

        bool threw = false;
        try {
            [[maybe_unused]] auto x = std::format("{:z}", command::iac);
        } catch (const std::format_error&) {
            threw = true;
        }

        expect(eq(threw, true));
    };
};

int main() {}
