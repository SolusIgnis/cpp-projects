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

#include <asio.hpp>

import std;
import ut;

import net.asio_concepts; // Module under test

using namespace ut;
using namespace net::asio_concepts;

// ─────────────────────────────────────────────────────────────────────────────
// Minimal dummy types for negative testing (intentionally fail concepts)
// ─────────────────────────────────────────────────────────────────────────────

struct NotABufferSequence {};

struct NotACompletionToken {};

struct BadSocketOption {
    int value() const { return 42; }  // wrong return type for most cases
};

struct NoExecutorType {
    // missing get_executor()
};

struct FakeWaitableNoWait {
    // missing wait() / async_wait()
};

// ─────────────────────────────────────────────────────────────────────────────
// Main test suite
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    "buffers"_test = [] {
        expect(true_b == AsioMutableBufferSequence<asio::mutable_buffer>);
        expect(true_b == AsioMutableBufferSequence<std::array<asio::mutable_buffer, 4>>);
        expect(true_b == AsioMutableBufferSequence<std::vector<asio::mutable_buffer>>);

        expect(true_b == AsioConstBufferSequence<asio::const_buffer>);
        expect(true_b == AsioConstBufferSequence<std::span<const asio::const_buffer>>);

        expect(false_b == AsioMutableBufferSequence<NotABufferSequence>);
        expect(false_b == AsioConstBufferSequence<NotABufferSequence>);
    };

    "tokens"_test = [] {
        expect(true_b == AsioReadToken<decltype(asio::use_awaitable)>);
        expect(true_b == AsioReadToken<decltype(asio::detached)>);
        expect(true_b == AsioReadToken<decltype(asio::use_future)>);

        expect(true_b == AsioWriteToken<decltype(asio::use_awaitable)>);
        expect(true_b == AsioConnectToken<decltype(asio::detached)>);

        expect(false_b == AsioReadToken<NotACompletionToken>);
    };

    "socket_options"_test = [] {
        using broadcast   = asio::socket_base::broadcast;
        using linger      = asio::socket_base::linger;
        using recv_buf    = asio::socket_base::receive_buffer_size;
        using join_group  = asio::ip::multicast::join_group;

        expect(true_b == BooleanSocketOption<broadcast>);
        expect(true_b == CompositeSocketOption<linger>);
        expect(true_b == IntegralSocketOption<recv_buf>);
        expect(true_b == AsioAddressibleSocketOption<join_group>);

        expect(true_b == SocketOption<broadcast>);
        expect(true_b == SocketOption<linger>);

        expect(true_b == SocketOptionProvider<asio::socket_base>);

        using tcp_socket = asio::ip::tcp::socket;
        expect(true_b == SocketOptionGetter<tcp_socket>);
        expect(true_b == SocketOptionSetter<tcp_socket>);

        expect(false_b == BooleanSocketOption<BadSocketOption>);
    };

    "io_flags_executor"_test = [] {
        using tcp_socket = asio::ip::tcp::socket;

        expect(true_b == IOController<tcp_socket>);
        expect(true_b == MessageFlagProvider<tcp_socket>);
        expect(true_b == AsioExecutorProvider<tcp_socket>);
        expect(true_b == AsioExecutorAssociated<tcp_socket>);

        expect(false_b == AsioExecutorAssociated<NoExecutorType>);
    };

    "streams"_test = [] {
        using tcp_socket = asio::ip::tcp::socket;

        expect(true_b == AsioAsyncReadStream<tcp_socket>);
        expect(true_b == AsioSyncReadStream<tcp_socket>);
        expect(true_b == AsioAsyncWriteStream<tcp_socket>);
        expect(true_b == AsioSyncWriteStream<tcp_socket>);

        expect(true_b == AsioStream<tcp_socket>);

        expect(false_b == AsioAsyncReadStream<NoExecutorType>);
    };

    "waitables"_test = [] {
        using timer      = asio::steady_timer;
        using tcp_socket = asio::ip::tcp::socket;

        expect(true_b == AsioAsyncTimedWaitable<timer>);
        expect(true_b == AsioSyncTimedWaitable<timer>);

        expect(true_b == AsioAsyncActivityWaitable<tcp_socket>);
        expect(true_b == AsioSyncActivityWaitable<tcp_socket>);

        expect(false_b == AsioAsyncTimedWaitable<FakeWaitableNoWait>);
    };

    "transmission"_test = [] {
        using tcp_socket = asio::ip::tcp::socket;

        expect(true_b == AsioAsyncSender<tcp_socket>);
        expect(true_b == AsioSyncSender<tcp_socket>);
        expect(true_b == AsioAsyncReceiver<tcp_socket>);
        expect(true_b == AsioSyncReceiver<tcp_socket>);

        expect(true_b == HasAtMark<tcp_socket>);
        expect(true_b == HasAvailable<tcp_socket>);
    };

    "lifecycle"_test = [] {
        using tcp_socket = asio::ip::tcp::socket;

        expect(true_b == CancellableResource<tcp_socket>);
        expect(true_b == ClosableResource<tcp_socket>);
        expect(true_b == EndpointProvider<tcp_socket>);
    };

    "connection"_test = [] {
        using tcp_socket = asio::ip::tcp::socket;

        expect(true_b == AsioAsyncConnectable<tcp_socket>);
        expect(true_b == AsioSyncConnectable<tcp_socket>);
        expect(true_b == NativeSocketWrapper<tcp_socket>);
    };

    "layering"_test = [] {
        using tcp_socket = asio::ip::tcp::socket;
        using ssl_stream = asio::ssl::stream<tcp_socket>;

        expect(true_b == LayerableObject<tcp_socket>);
        expect(true_b == LayerableObject<ssl_stream>);

        expect(true_b == LayeredObject<ssl_stream>);

        expect(true_b == AsioLayerableSocket<ssl_stream>);
        expect(true_b == AsioLayerableStreamSocket<ssl_stream>);
    };

    "umbrella"_test = [] {
        using tcp_socket = asio::ip::tcp::socket;
        using ssl_stream = asio::ssl::stream<tcp_socket>;

        expect(true_b == AsioSocket<tcp_socket>);
        expect(true_b == AsioStreamSocket<tcp_socket>);

        expect(true_b == AsioLayerableStreamSocket<ssl_stream>);

        expect(false_b == AsioSocket<NotABufferSequence>);
        expect(false_b == AsioStreamSocket<BadSocketOption>);
    };

    // All tests are now registered — ut will run them and report results
}
