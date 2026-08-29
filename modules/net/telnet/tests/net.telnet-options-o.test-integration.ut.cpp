// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:options

import net.telnet;
import ut;
import std;

using namespace ut;

namespace {
//NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
suite net_telnet_option_registry_tests = [] mutable {
    using net::telnet::option;
    using net::telnet::option_registry;

    "registry get and has work"_test = [] mutable {
        option_registry registry{
            option{option::id_num::binary, "Binary"},
            option{  option::id_num::echo,   "Echo"},
        };

        expect(eq(registry.has(option::id_num::binary), true));
        expect(eq(registry.has(option::id_num::status), false));

        auto opt = registry.get(option::id_num::echo);
        expect(eq(opt.has_value(), true));
        expect(eq(opt->get_name(), std::string{"Echo"}));
    };

    "upsert inserts new option"_test = [] mutable {
        option_registry registry{};

        const auto& inserted = registry.upsert(option{option::id_num::binary, "Binary"});

        expect(eq((inserted.get_id() == option::id_num::binary), true));
        expect(eq(registry.has(option::id_num::binary), true));
    };

    "upsert replaces existing option"_test = [] mutable {
        option_registry registry{
            option{option::id_num::binary, "OldName"},
        };

        registry.upsert(option{option::id_num::binary, "NewName"});

        auto opt = registry.get(option::id_num::binary);
        expect(eq(opt.has_value(), true));
        expect(eq(opt->get_name(), std::string{"NewName"}));
    };

    "upsert with error_code does not set error on success"_test = [] mutable {
        option_registry registry{};

        std::error_code ec;
        registry.upsert(option{option::id_num::binary, "Binary"}, ec);

        expect(eq(ec.value(), 0));
    };
};

//NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
suite net_telnet_option_formatter_tests = [] mutable {
    using net::telnet::option;

    "default format prints hex and name"_test = [] mutable {
        option opt{option::id_num::binary, "Binary"};

        auto s = std::format("{}", opt);

        expect(eq(s, std::string{"0x00 (Binary)"}));
    };

    "name-only format"_test = [] mutable {
        option opt{option::id_num::binary, "Binary"};

        auto s = std::format("{:n}", opt);

        expect(eq(s, std::string{"Binary"}));
    };

    "hex-only format"_test = [] mutable {
        option opt{option::id_num::binary, "Binary"};

        auto s = std::format("{:x}", opt);

        expect(eq(s, std::string{"0x00"}));
    };

    "empty name formats as unknown"_test = [] mutable {
        option opt{option::id_num::binary};

        auto s = std::format("{:n}", opt);

        expect(eq(s, std::string{"unknown"}));
    };
};
} //namespace

int main() {}
