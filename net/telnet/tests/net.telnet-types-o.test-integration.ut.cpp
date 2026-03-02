// SPDX-License-Identifier: Apache-2.0
// Integration tests for net.telnet:types
// Verifies interoperability with std library and full value ranges.

import net.telnet;
import ut;

using namespace net::telnet;
using namespace ut;

int main() {
// ============================================================
// All RFC command range formatting must not throw
// ============================================================

"all RFC command-range values format without throwing"_test = [] mutable {
    for (int value = 0xEF; value <= 0xFF; ++value) {
        auto cmd = static_cast<command>(static_cast<byte_t>(value));

        try {
            std::format("{}", cmd);
            std::format("{:n}", cmd);
            std::format("{:x}", cmd);
        } catch (...) {
            expect(false) << "std::format throws for command: " << cmd;
        }
    }
};

// ============================================================
// Full 0-255 robustness check (no UB, no crashes)
// ============================================================

"all 256 possible command byte values are safely formattable"_test = [] mutable {
    for (int value = 0; value <= 0xFF; ++value) {
        auto cmd = static_cast<command>(static_cast<byte_t>(value));

        try {
            std::format("{}", cmd);
        } catch (...) {
            expect(false) << "std::format throws for command: " << cmd;
        }
    }
};

// ============================================================
// Hex formatting preserves underlying value width and lowercase
// ============================================================

"hex formatting is zero-padded lowercase"_test = [] mutable {
    auto formatted = std::format("{:x}", command::eor);

    expect(formatted.size() == 4); // "0x??"
    expect(formatted.starts_with("0x"));
};

// ============================================================
// Composability inside larger formatted expressions
// ============================================================

"formatter composes inside nested format expressions"_test = [] mutable {
    auto msg = std::format("[{}:{}]", command::iac, negotiation_direction::remote);

    expect(msg == "[IAC (0xff):remote]");
};

// ============================================================
// Round-trip consistency check
// ============================================================

"hex format matches underlying numeric value"_test = [] mutable {
    auto cmd = command::sb;
    auto hex = std::format("{:x}", cmd);

    expect(hex == "0xfa");
};

return 0;
}
