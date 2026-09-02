// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
/**
 * @file net.asio_concepts.test.ut.cpp
 * @brief Runtime unit tests for net::asio_concepts using qlibs/ut
 *
 * All checks are performed at run-time → failed assertions are reported
 * but do NOT abort the test suite. Suitable for CI red/green reporting.
 *
 * Build example (adjust paths):
 *   c++ -std=c++23 -I/path/to/ut/single-header -I/path/to/asio \
 *       test_net.asio_concepts.cpp -o test_concepts && ./test_concepts
 */

//NOLINTBEGIN: Ignore tidy for dependency includes.
#include <asio.hpp>
#include <asio/ssl.hpp>
//NOLINTEND

import std;
import ut;

import net.asio_concepts; // Code under test.

using namespace ut;                 //NOLINT(google-build-using-namespace)
using namespace net::asio_concepts; //NOLINT(google-build-using-namespace)

// ─────────────────────────────────────────────────────────────────────────────
// Minimal dummy types for negative testing (intentionally fail concepts)
// ─────────────────────────────────────────────────────────────────────────────

struct not_a_buffer_sequence {};

struct not_a_completion_token {};

struct bad_socket_option {
    //NOLINTNEXTLINE(readability-convert-member-functions-to-static): Test fixture.
    [[nodiscard]] int value() const { return {}; } // wrong return type for most cases
};

struct no_executor_type {
    // missing get_executor()
};

struct fake_waitable_no_wait {
    // missing wait() / async_wait()
};

// ─────────────────────────────────────────────────────────────────────────────
// Main test suite
// ─────────────────────────────────────────────────────────────────────────────

int main()
{
    "buffers"_test = [] mutable {
        expect(_b{true} == asio_mutable_buffer_sequence<asio::mutable_buffer>);
        expect(_b{true} == asio_mutable_buffer_sequence<std::array<asio::mutable_buffer, 4>>);
        expect(_b{true} == asio_mutable_buffer_sequence<std::vector<asio::mutable_buffer>>);

        expect(_b{true} == asio_const_buffer_sequence<asio::const_buffer>);
        expect(_b{true} == asio_const_buffer_sequence<std::span<const asio::const_buffer>>);

        expect(_b{false} == asio_mutable_buffer_sequence<not_a_buffer_sequence>);
        expect(_b{false} == asio_const_buffer_sequence<not_a_buffer_sequence>);
    };

    "tokens"_test = [] mutable {
        expect(_b{true} == asio_read_token<decltype(asio::use_awaitable)>);
        expect(_b{true} == asio_read_token<decltype(asio::detached)>);
        expect(_b{true} == asio_read_token<decltype(asio::use_future)>);

        expect(_b{true} == asio_write_token<decltype(asio::use_awaitable)>);
        expect(_b{true} == asio_connect_token<decltype(asio::detached)>);

        expect(_b{false} == asio_read_token<not_a_completion_token>);
    };

    "socket_options"_test = [] mutable {
        using broadcast = asio::socket_base::broadcast;
        using linger    = asio::socket_base::linger;
        using recv_buf  = asio::socket_base::receive_buffer_size;
        using asio::ip::multicast::join_group;

        expect(_b{true} == boolean_socket_option<broadcast>);
        expect(_b{true} == composite_socket_option<linger>);
        expect(_b{true} == integral_socket_option<recv_buf>);
        expect(_b{true} == asio_addressible_socket_option<join_group>);

        expect(_b{true} == socket_option<broadcast>);
        expect(_b{true} == socket_option<linger>);

        expect(_b{true} == socket_option_provider<asio::socket_base>);

        using tcp_socket = asio::ip::tcp::socket;
        expect(_b{true} == socket_option_getter<tcp_socket>);
        expect(_b{true} == socket_option_setter<tcp_socket>);

        expect(_b{false} == boolean_socket_option<bad_socket_option>);
    };

    "io_flags_executor"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == io_controller<tcp_socket>);
        expect(_b{true} == message_flag_provider<tcp_socket>);
        expect(_b{true} == asio_executor_provider<tcp_socket>);
        expect(_b{true} == asio_executor_associated<tcp_socket>);

        expect(_b{false} == asio_executor_associated<no_executor_type>);
    };

    "streams"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == asio_async_read_stream<tcp_socket>);
        expect(_b{true} == asio_sync_read_stream<tcp_socket>);
        expect(_b{true} == asio_async_write_stream<tcp_socket>);
        expect(_b{true} == asio_sync_write_stream<tcp_socket>);

        expect(_b{true} == asio_stream<tcp_socket>);

        expect(_b{false} == asio_async_read_stream<no_executor_type>);
    };

    "waitables"_test = [] mutable {
        using timer      = asio::steady_timer;
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == asio_async_timed_waitable<timer>);
        expect(_b{true} == asio_sync_timed_waitable<timer>);

        expect(_b{true} == asio_async_activity_waitable<tcp_socket>);
        expect(_b{true} == asio_sync_activity_waitable<tcp_socket>);

        expect(_b{false} == asio_async_timed_waitable<fake_waitable_no_wait>);
    };

    "transmission"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == asio_async_sender<tcp_socket>);
        expect(_b{true} == asio_sync_sender<tcp_socket>);
        expect(_b{true} == asio_async_receiver<tcp_socket>);
        expect(_b{true} == asio_sync_receiver<tcp_socket>);

        expect(_b{true} == has_at_mark<tcp_socket>);
        expect(_b{true} == has_available<tcp_socket>);
    };

    "lifecycle"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == cancellable_resource<tcp_socket>);
        expect(_b{true} == closable_resource<tcp_socket>);
        expect(_b{true} == endpoint_provider<tcp_socket>);
    };

    "connection"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == asio_async_connectable<tcp_socket>);
        expect(_b{true} == asio_sync_connectable<tcp_socket>);
        expect(_b{true} == native_socket_wrapper<tcp_socket>);
    };

    "layering"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == layerable_object<tcp_socket>);

#ifdef ASIO_HAS_OPENSSL
        using ssl_stream = asio::ssl::stream<tcp_socket>;
        expect(_b{true} == layerable_object<ssl_stream>);

        expect(_b{true} == layered_object<ssl_stream>);

        expect(_b{true} == asio_layerable_socket<ssl_stream>);
        expect(_b{true} == asio_layerable_stream_socket<ssl_stream>);
#endif
    };

    "umbrella"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == asio_socket<tcp_socket>);
        expect(_b{true} == asio_stream_socket<tcp_socket>);

#ifdef ASIO_HAS_OPENSSL
        using ssl_stream = asio::ssl::stream<tcp_socket>;
        expect(_b{true} == asio_layerable_stream_socket<ssl_stream>);
#endif

        expect(_b{false} == asio_socket<not_a_buffer_sequence>);
        expect(_b{false} == asio_stream_socket<bad_socket_option>);
    };

    // All tests are now registered — ut will run them and report results
}
