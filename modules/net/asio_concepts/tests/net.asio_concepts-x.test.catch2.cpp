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

struct NotABufferSequence {};

struct NotACompletionToken {};

struct BadSocketOption {
    int value() const { return 42; } // deliberately wrong for most option concepts
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

TEST_CASE("buffers", "[concepts][buffers]")
{
    using mb_array = std::array<asio::mutable_buffer, 4>;
    CONCEPT_CHECK(AsioMutableBufferSequence<asio::mutable_buffer>);
    CONCEPT_CHECK(AsioMutableBufferSequence<mb_array>);
    CONCEPT_CHECK(AsioMutableBufferSequence<std::vector<asio::mutable_buffer>>);

    CONCEPT_CHECK(AsioConstBufferSequence<asio::const_buffer>);
    CONCEPT_CHECK(AsioConstBufferSequence<std::span<const asio::const_buffer>>);

    CHECK_FALSE(AsioMutableBufferSequence<NotABufferSequence>);
    CHECK_FALSE(AsioConstBufferSequence<NotABufferSequence>);
}

TEST_CASE("tokens", "[concepts][tokens]")
{
    CONCEPT_CHECK(AsioReadToken<decltype(asio::use_awaitable)>);
    CONCEPT_CHECK(AsioReadToken<decltype(asio::detached)>);
    CONCEPT_CHECK(AsioReadToken<decltype(asio::use_future)>);

    CONCEPT_CHECK(AsioWriteToken<decltype(asio::use_awaitable)>);
    CONCEPT_CHECK(AsioConnectToken<decltype(asio::detached)>);

    CHECK_FALSE(AsioReadToken<NotACompletionToken>);
}

TEST_CASE("socket_options", "[concepts][socket_options]")
{
    using broadcast  = asio::socket_base::broadcast;
    using linger     = asio::socket_base::linger;
    using recv_buf   = asio::socket_base::receive_buffer_size;
    using join_group = asio::ip::multicast::join_group;

    CONCEPT_CHECK(BooleanSocketOption<broadcast>);
    CONCEPT_CHECK(CompositeSocketOption<linger>);
    CONCEPT_CHECK(IntegralSocketOption<recv_buf>);
    CONCEPT_CHECK(AsioAddressibleSocketOption<join_group>);

    CONCEPT_CHECK(SocketOption<broadcast>);
    CONCEPT_CHECK(SocketOption<linger>);

    CONCEPT_CHECK(SocketOptionProvider<asio::socket_base>);

    using tcp_socket = asio::ip::tcp::socket;
    CONCEPT_CHECK(SocketOptionGetter<tcp_socket>);
    CONCEPT_CHECK(SocketOptionSetter<tcp_socket>);

    CHECK_FALSE(BooleanSocketOption<BadSocketOption>);
}

TEST_CASE("io_flags_executor", "[concepts][io][flags][executor]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(IOController<tcp_socket>);
    CONCEPT_CHECK(MessageFlagProvider<tcp_socket>);
    CONCEPT_CHECK(AsioExecutorProvider<tcp_socket>);
    CONCEPT_CHECK(AsioExecutorAssociated<tcp_socket>);

    CHECK_FALSE(AsioExecutorAssociated<NoExecutorType>);
}

TEST_CASE("streams", "[concepts][streams]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(AsioAsyncReadStream<tcp_socket>);
    CONCEPT_CHECK(AsioSyncReadStream<tcp_socket>);
    CONCEPT_CHECK(AsioAsyncWriteStream<tcp_socket>);
    CONCEPT_CHECK(AsioSyncWriteStream<tcp_socket>);

    CONCEPT_CHECK(AsioStream<tcp_socket>);

    CHECK_FALSE(AsioAsyncReadStream<NoExecutorType>);
}

TEST_CASE("waitables", "[concepts][waitables]")
{
    using timer      = asio::steady_timer;
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(AsioAsyncTimedWaitable<timer>);
    CONCEPT_CHECK(AsioSyncTimedWaitable<timer>);

    CONCEPT_CHECK(AsioAsyncActivityWaitable<tcp_socket>);
    CONCEPT_CHECK(AsioSyncActivityWaitable<tcp_socket>);

    CHECK_FALSE(AsioAsyncTimedWaitable<FakeWaitableNoWait>);
}

TEST_CASE("transmission", "[concepts][transmission]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(AsioAsyncSender<tcp_socket>);
    CONCEPT_CHECK(AsioSyncSender<tcp_socket>);
    CONCEPT_CHECK(AsioAsyncReceiver<tcp_socket>);
    CONCEPT_CHECK(AsioSyncReceiver<tcp_socket>);

    CONCEPT_CHECK(HasAtMark<tcp_socket>);
    CONCEPT_CHECK(HasAvailable<tcp_socket>);
}

TEST_CASE("lifecycle", "[concepts][lifecycle]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(CancellableResource<tcp_socket>);
    CONCEPT_CHECK(ClosableResource<tcp_socket>);
    CONCEPT_CHECK(EndpointProvider<tcp_socket>);
}

TEST_CASE("connection", "[concepts][connection]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(AsioAsyncConnectable<tcp_socket>);
    CONCEPT_CHECK(AsioSyncConnectable<tcp_socket>);
    CONCEPT_CHECK(NativeSocketWrapper<tcp_socket>);
}

TEST_CASE("layering", "[concepts][layering]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(LayerableObject<tcp_socket>);

#ifdef ASIO_HAS_OPENSSL
    using ssl_stream = asio::ssl::stream<tcp_socket>;

    CONCEPT_CHECK(LayerableObject<ssl_stream>);

    CONCEPT_CHECK(LayeredObject<ssl_stream>);

    CONCEPT_CHECK(AsioLayerableSocket<ssl_stream>);
    CONCEPT_CHECK(AsioLayerableStreamSocket<ssl_stream>);
#endif
}

TEST_CASE("umbrella concepts", "[concepts][umbrella]")
{
    using tcp_socket = asio::ip::tcp::socket;

    CONCEPT_CHECK(AsioSocket<tcp_socket>);
    CONCEPT_CHECK(AsioStreamSocket<tcp_socket>);

#ifdef ASIO_HAS_OPENSSL
    using ssl_stream = asio::ssl::stream<tcp_socket>;
    CONCEPT_CHECK(AsioLayerableStreamSocket<ssl_stream>);
#endif

    CHECK_FALSE(AsioSocket<NotABufferSequence>);
    CHECK_FALSE(AsioStreamSocket<BadSocketOption>);
}
