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

#include <catch2/catch_test_macros.hpp>   // Core macros: TEST_CASE, REQUIRE, CHECK, ...
#include <catch2/catch_template_test_macros.hpp>  // TEMPLATE_TEST_CASE if needed later

#include <asio.hpp>
#include <asio/ssl.hpp>     // for ssl::stream layering tests

import net.asio_concepts;   // your module
import std;

using namespace net::asio_concepts;

// ─────────────────────────────────────────────────────────────────────────────
// Minimal dummy types for negative testing
// ─────────────────────────────────────────────────────────────────────────────

struct NotABufferSequence {};

struct NotACompletionToken {};

struct BadSocketOption {
    int value() const { return 42; }  // deliberately wrong for most option concepts
};

struct NoExecutorType {
    // missing get_executor()
};

struct FakeWaitableNoWait {
    // missing wait() / async_wait()
};

// ─────────────────────────────────────────────────────────────────────────────
// Tests
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("buffers", "[concepts][buffers]") {
    REQUIRE(AsioMutableBufferSequence<asio::mutable_buffer>);
    REQUIRE(AsioMutableBufferSequence<std::array<asio::mutable_buffer, 4>>);
    REQUIRE(AsioMutableBufferSequence<std::vector<asio::mutable_buffer>>);

    REQUIRE(AsioConstBufferSequence<asio::const_buffer>);
    REQUIRE(AsioConstBufferSequence<std::span<const asio::const_buffer>>);

    REQUIRE_FALSE(AsioMutableBufferSequence<NotABufferSequence>);
    REQUIRE_FALSE(AsioConstBufferSequence<NotABufferSequence>);
}

TEST_CASE("tokens", "[concepts][tokens]") {
    REQUIRE(AsioReadToken<asio::use_awaitable>);
    REQUIRE(AsioReadToken<asio::detached>);
    REQUIRE(AsioReadToken<asio::use_future>);

    REQUIRE(AsioWriteToken<asio::use_awaitable>);
    REQUIRE(AsioConnectToken<asio::detached>);

    REQUIRE_FALSE(AsioReadToken<NotACompletionToken>);
}

TEST_CASE("socket_options", "[concepts][socket_options]") {
    using broadcast   = asio::socket_base::broadcast;
    using linger      = asio::socket_base::linger;
    using recv_buf    = asio::socket_base::receive_buffer_size;
    using join_group  = asio::ip::multicast::join_group;

    REQUIRE(BooleanSocketOption<broadcast>);
    REQUIRE(CompositeSocketOption<linger>);
    REQUIRE(IntegralSocketOption<recv_buf>);
    REQUIRE(AsioAddressibleSocketOption<join_group>);

    REQUIRE(SocketOption<broadcast>);
    REQUIRE(SocketOption<linger>);

    REQUIRE(SocketOptionProvider<asio::socket_base>);

    using tcp_socket = asio::ip::tcp::socket;
    REQUIRE(SocketOptionGetter<tcp_socket>);
    REQUIRE(SocketOptionSetter<tcp_socket>);

    REQUIRE_FALSE(BooleanSocketOption<BadSocketOption>);
}

TEST_CASE("io_flags_executor", "[concepts][io][flags][executor]") {
    using tcp_socket = asio::ip::tcp::socket;

    REQUIRE(IOController<tcp_socket>);
    REQUIRE(MessageFlagProvider<tcp_socket>);
    REQUIRE(AsioExecutorProvider<tcp_socket>);
    REQUIRE(AsioExecutorAssociated<tcp_socket>);

    REQUIRE_FALSE(AsioExecutorAssociated<NoExecutorType>);
}

TEST_CASE("streams", "[concepts][streams]") {
    using tcp_socket = asio::ip::tcp::socket;

    REQUIRE(AsioAsyncReadStream<tcp_socket>);
    REQUIRE(AsioSyncReadStream<tcp_socket>);
    REQUIRE(AsioAsyncWriteStream<tcp_socket>);
    REQUIRE(AsioSyncWriteStream<tcp_socket>);

    REQUIRE(AsioStream<tcp_socket>);

    REQUIRE_FALSE(AsioAsyncReadStream<NoExecutorType>);
}

TEST_CASE("waitables", "[concepts][waitables]") {
    using timer      = asio::steady_timer;
    using tcp_socket = asio::ip::tcp::socket;

    REQUIRE(AsioAsyncTimedWaitable<timer>);
    REQUIRE(AsioSyncTimedWaitable<timer>);

    REQUIRE(AsioAsyncActivityWaitable<tcp_socket>);
    REQUIRE(AsioSyncActivityWaitable<tcp_socket>);

    REQUIRE_FALSE(AsioAsyncTimedWaitable<FakeWaitableNoWait>);
}

TEST_CASE("transmission", "[concepts][transmission]") {
    using tcp_socket = asio::ip::tcp::socket;

    REQUIRE(AsioAsyncSender<tcp_socket>);
    REQUIRE(AsioSyncSender<tcp_socket>);
    REQUIRE(AsioAsyncReceiver<tcp_socket>);
    REQUIRE(AsioSyncReceiver<tcp_socket>);

    REQUIRE(HasAtMark<tcp_socket>);
    REQUIRE(HasAvailable<tcp_socket>);
}

TEST_CASE("lifecycle", "[concepts][lifecycle]") {
    using tcp_socket = asio::ip::tcp::socket;

    REQUIRE(CancellableResource<tcp_socket>);
    REQUIRE(ClosableResource<tcp_socket>);
    REQUIRE(EndpointProvider<tcp_socket>);
}

TEST_CASE("connection", "[concepts][connection]") {
    using tcp_socket = asio::ip::tcp::socket;

    REQUIRE(AsioAsyncConnectable<tcp_socket>);
    REQUIRE(AsioSyncConnectable<tcp_socket>);
    REQUIRE(NativeSocketWrapper<tcp_socket>);
}

TEST_CASE("layering", "[concepts][layering]") {
    using tcp_socket = asio::ip::tcp::socket;
    using ssl_stream = asio::ssl::stream<tcp_socket>;

    REQUIRE(LayerableObject<tcp_socket>);
    REQUIRE(LayerableObject<ssl_stream>);

    REQUIRE(LayeredObject<ssl_stream>);

    REQUIRE(AsioLayerableSocket<ssl_stream>);
    REQUIRE(AsioLayerableStreamSocket<ssl_stream>);
}

TEST_CASE("umbrella concepts", "[concepts][umbrella]") {
    using tcp_socket = asio::ip::tcp::socket;
    using ssl_stream = asio::ssl::stream<tcp_socket>;

    REQUIRE(AsioSocket<tcp_socket>);
    REQUIRE(AsioStreamSocket<tcp_socket>);

    REQUIRE(AsioLayerableStreamSocket<ssl_stream>);

    REQUIRE_FALSE(AsioSocket<NotABufferSequence>);
    REQUIRE_FALSE(AsioStreamSocket<BadSocketOption>);
}

TEST_CASE("completion signatures", "[concepts][signatures]") {
    using Sig = void(std::error_code, std::size_t);

    REQUIRE(std::same_as<asio_read_completion_signature, Sig>);
    REQUIRE(std::same_as<asio_write_completion_signature, Sig>);
    REQUIRE(std::same_as<asio_wait_completion_signature, void(std::error_code)>);
}
