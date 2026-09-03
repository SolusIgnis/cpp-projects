// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
/**
 * @file net.asio_concepts.test-grok.catch2.cpp
 * @brief Runtime unit tests for net::asio_concepts using Catch2 v3+
 *
 * All checks are runtime → failures are reported but do NOT abort the suite.
 * Suitable for CI (red/green reporting).
 *
 * Build example (single-file style):
 *   c++ -std=c++23 -I/path/to/asio -I/path/to/catch2/single-include \
 *       test_net.asio_concepts.cpp -o test_concepts && ./test_concepts
 *
 * Or with CMake + Catch2 installed:
 *   find_package(Catch2 3 REQUIRED)
 *   target_link_libraries(your_test Catch2::Catch2WithMain)
 */

//NOLINTBEGIN: Ignore tidy for dependency includes.
#include <catch2/catch_test_macros.hpp>          // Core macros: TEST_CASE, CHECK, ...
#include <catch2/catch_template_test_macros.hpp> // TEMPLATE_TEST_CASE if needed later

#include <asio.hpp>
#include <asio/ssl.hpp>
//NOLINTEND

import net.asio_concepts; // Code under test.
import std;

using namespace net::asio_concepts; //NOLINT(google-build-using-namespace)

#define CONCEPT_CHECK(ConceptCheckAssertion)  \
    {                                         \
        static_assert(ConceptCheckAssertion); \
        CHECK(ConceptCheckAssertion);         \
    }

// ─────────────────────────────────────────────────────────────────────────────
// Minimal dummy types for negative testing
// ─────────────────────────────────────────────────────────────────────────────

struct not_a_buffer_sequence {};

struct not_a_completion_token {};

struct bad_socket_option {
    int value() const { return 42; } // deliberately wrong for most option concepts
};

struct no_executor_type {
    // missing get_executor()
};

struct fake_waitable_no_wait {
    // missing wait() / async_wait()
};

// ─────────────────────────────────────────────────────────────────────────────
// Tests
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("buffers", "[concepts][buffers]")
{
    using mb_array = std::array<asio::mutable_buffer, 4>;
    CONCEPT_CHECK(asio_mutable_buffer_sequence<asio::mutable_buffer>);
    CONCEPT_CHECK(asio_mutable_buffer_sequence<mb_array>);
    CONCEPT_CHECK(asio_mutable_buffer_sequence<std::vector<asio::mutable_buffer>>);

    CONCEPT_CHECK(asio_const_buffer_sequence<asio::const_buffer>);
    CONCEPT_CHECK(asio_const_buffer_sequence<std::span<const asio::const_buffer>>);

    CHECK_FALSE(asio_mutable_buffer_sequence<not_a_buffer_sequence>);
    CHECK_FALSE(asio_const_buffer_sequence<not_a_buffer_sequence>);
}

TEST_CASE("tokens", "[concepts][tokens]")
{
    CONCEPT_CHECK(asio_read_token<decltype(asio::use_awaitable)>);
    CONCEPT_CHECK(asio_read_token<decltype(asio::detached)>);
    CONCEPT_CHECK(asio_read_token<decltype(asio::use_future)>);

    CONCEPT_CHECK(asio_write_token<decltype(asio::use_awaitable)>);
    CONCEPT_CHECK(asio_connect_token<decltype(asio::detached)>);

    CHECK_FALSE(asio_read_token<not_a_completion_token>);
}

TEST_CASE("socket_options", "[concepts][socket_options]")
{
    using broadcast  = asio::socket_base::broadcast;
    using linger     = asio::socket_base::linger;
    using recv_buf   = asio::socket_base::receive_buffer_size;
    using join_group = asio::ip::multicast::join_group;

    CONCEPT_CHECK(boolean_socket_option<broadcast>);
    CONCEPT_CHECK(composite_socket_option<linger>);
    CONCEPT_CHECK(integral_socket_option<recv_buf>);
    CONCEPT_CHECK(asio_addressable_socket_option<join_group>);

    CONCEPT_CHECK(socket_option<broadcast>);
    CONCEPT_CHECK(socket_option<linger>);

    CONCEPT_CHECK(socket_option_provider<asio::socket_base>);

    using tcp_socket = asio::ip::tcp::socket;
    CONCEPT_CHECK(socket_option_getter<tcp_socket>);
    CONCEPT_CHECK(socket_option_setter<tcp_socket>);

    CHECK_FALSE(boolean_socket_option<bad_socket_option>);
}

TEST_CASE("io_flags_executor", "[concepts][io][flags][executor]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(io_controller<tcp_socket>);
    CONCEPT_CHECK(message_flag_provider<tcp_socket>);
    CONCEPT_CHECK(asio_executor_provider<tcp_socket>);
    CONCEPT_CHECK(asio_executor_associated<tcp_socket>);

    CHECK_FALSE(asio_executor_associated<no_executor_type>);
}

TEST_CASE("streams", "[concepts][streams]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(asio_async_read_stream<tcp_socket>);
    CONCEPT_CHECK(asio_sync_read_stream<tcp_socket>);
    CONCEPT_CHECK(asio_async_write_stream<tcp_socket>);
    CONCEPT_CHECK(asio_sync_write_stream<tcp_socket>);

    CONCEPT_CHECK(asio_stream<tcp_socket>);

    CHECK_FALSE(asio_async_read_stream<no_executor_type>);
}

TEST_CASE("waitables", "[concepts][waitables]")
{
    using timer      = asio::steady_timer;
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(asio_async_timed_waitable<timer>);
    CONCEPT_CHECK(asio_sync_timed_waitable<timer>);

    CONCEPT_CHECK(asio_async_activity_waitable<tcp_socket>);
    CONCEPT_CHECK(asio_sync_activity_waitable<tcp_socket>);

    CHECK_FALSE(asio_async_timed_waitable<fake_waitable_no_wait>);
}

TEST_CASE("transmission", "[concepts][transmission]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(asio_async_sender<tcp_socket>);
    CONCEPT_CHECK(asio_sync_sender<tcp_socket>);
    CONCEPT_CHECK(asio_async_receiver<tcp_socket>);
    CONCEPT_CHECK(asio_sync_receiver<tcp_socket>);

    CONCEPT_CHECK(has_at_mark<tcp_socket>);
    CONCEPT_CHECK(has_available<tcp_socket>);
}

TEST_CASE("lifecycle", "[concepts][lifecycle]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(cancellable_resource<tcp_socket>);
    CONCEPT_CHECK(closable_resource<tcp_socket>);
    CONCEPT_CHECK(endpoint_provider<tcp_socket>);
}

TEST_CASE("connection", "[concepts][connection]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(asio_async_connectable<tcp_socket>);
    CONCEPT_CHECK(asio_sync_connectable<tcp_socket>);
    CONCEPT_CHECK(native_socket_wrapper<tcp_socket>);
}

TEST_CASE("layering", "[concepts][layering]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(layerable_object<tcp_socket>);

#ifdef ASIO_HAS_OPENSSL
    using ssl_stream = asio::ssl::stream<tcp_socket>;

    CONCEPT_CHECK(layerable_object<ssl_stream>);

    CONCEPT_CHECK(layered_object<ssl_stream>);

    CONCEPT_CHECK(asio_layerable_socket<ssl_stream>);
    CONCEPT_CHECK(asio_layerable_stream_socket<ssl_stream>);
#endif
}

TEST_CASE("umbrella concepts", "[concepts][umbrella]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(asio_socket<tcp_socket>);
    CONCEPT_CHECK(asio_stream_socket<tcp_socket>);

#ifdef ASIO_HAS_OPENSSL
    using ssl_stream = asio::ssl::stream<tcp_socket>;
    CONCEPT_CHECK(asio_layerable_stream_socket<ssl_stream>);
#endif

    CHECK_FALSE(asio_socket<not_a_buffer_sequence>);
    CHECK_FALSE(asio_stream_socket<bad_socket_option>);
}
