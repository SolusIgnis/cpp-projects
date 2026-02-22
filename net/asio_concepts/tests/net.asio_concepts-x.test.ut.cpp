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

using namespace ut; //NOLINT(google-build-using-namespace)
using namespace net::asio_concepts; //NOLINT(google-build-using-namespace)

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
    "buffers"_test = [] mutable {
        expect(_b{true} == AsioMutableBufferSequence<asio::mutable_buffer>);
        expect(_b{true} == AsioMutableBufferSequence<std::array<asio::mutable_buffer, 4>>);
        expect(_b{true} == AsioMutableBufferSequence<std::vector<asio::mutable_buffer>>);

        expect(_b{true} == AsioConstBufferSequence<asio::const_buffer>);
        expect(_b{true} == AsioConstBufferSequence<std::span<const asio::const_buffer>>);

        expect(_b{false} == AsioMutableBufferSequence<NotABufferSequence>);
        expect(_b{false} == AsioConstBufferSequence<NotABufferSequence>);
    };

    "tokens"_test = [] mutable {
        expect(_b{true} == AsioReadToken<decltype(asio::use_awaitable)>);
        expect(_b{true} == AsioReadToken<decltype(asio::detached)>);
        expect(_b{true} == AsioReadToken<decltype(asio::use_future)>);

        expect(_b{true} == AsioWriteToken<decltype(asio::use_awaitable)>);
        expect(_b{true} == AsioConnectToken<decltype(asio::detached)>);

        expect(_b{false} == AsioReadToken<NotACompletionToken>);
    };

    "socket_options"_test = [] mutable {
        using broadcast   = asio::socket_base::broadcast;
        using linger      = asio::socket_base::linger;
        using recv_buf    = asio::socket_base::receive_buffer_size;
        using join_group  = asio::ip::multicast::join_group;

        expect(_b{true} == BooleanSocketOption<broadcast>);
        expect(_b{true} == CompositeSocketOption<linger>);
        expect(_b{true} == IntegralSocketOption<recv_buf>);
        expect(_b{true} == AsioAddressibleSocketOption<join_group>);

        expect(_b{true} == SocketOption<broadcast>);
        expect(_b{true} == SocketOption<linger>);

        expect(_b{true} == SocketOptionProvider<asio::socket_base>);

        using tcp_socket = asio::ip::tcp::socket;
        expect(_b{true} == SocketOptionGetter<tcp_socket>);
        expect(_b{true} == SocketOptionSetter<tcp_socket>);

        expect(_b{false} == BooleanSocketOption<BadSocketOption>);
    };

    "io_flags_executor"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == IOController<tcp_socket>);
        expect(_b{true} == MessageFlagProvider<tcp_socket>);
        expect(_b{true} == AsioExecutorProvider<tcp_socket>);
        expect(_b{true} == AsioExecutorAssociated<tcp_socket>);

        expect(_b{false} == AsioExecutorAssociated<NoExecutorType>);
    };

    "streams"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == AsioAsyncReadStream<tcp_socket>);
        expect(_b{true} == AsioSyncReadStream<tcp_socket>);
        expect(_b{true} == AsioAsyncWriteStream<tcp_socket>);
        expect(_b{true} == AsioSyncWriteStream<tcp_socket>);

        expect(_b{true} == AsioStream<tcp_socket>);

        expect(_b{false} == AsioAsyncReadStream<NoExecutorType>);
    };

    "waitables"_test = [] mutable {
        using timer      = asio::steady_timer;
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == AsioAsyncTimedWaitable<timer>);
        expect(_b{true} == AsioSyncTimedWaitable<timer>);

        expect(_b{true} == AsioAsyncActivityWaitable<tcp_socket>);
        expect(_b{true} == AsioSyncActivityWaitable<tcp_socket>);

        expect(_b{false} == AsioAsyncTimedWaitable<FakeWaitableNoWait>);
    };

    "transmission"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == AsioAsyncSender<tcp_socket>);
        expect(_b{true} == AsioSyncSender<tcp_socket>);
        expect(_b{true} == AsioAsyncReceiver<tcp_socket>);
        expect(_b{true} == AsioSyncReceiver<tcp_socket>);

        expect(_b{true} == HasAtMark<tcp_socket>);
        expect(_b{true} == HasAvailable<tcp_socket>);
    };

    "lifecycle"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == CancellableResource<tcp_socket>);
        expect(_b{true} == ClosableResource<tcp_socket>);
        expect(_b{true} == EndpointProvider<tcp_socket>);
    };

    "connection"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == AsioAsyncConnectable<tcp_socket>);
        expect(_b{true} == AsioSyncConnectable<tcp_socket>);
        expect(_b{true} == NativeSocketWrapper<tcp_socket>);
    };

    "layering"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == LayerableObject<tcp_socket>);
        
        #ifdef ASIO_HAS_OPENSSL
        using ssl_stream = asio::ssl::stream<tcp_socket>;
        expect(_b{true} == LayerableObject<ssl_stream>);

        expect(_b{true} == LayeredObject<ssl_stream>);

        expect(_b{true} == AsioLayerableSocket<ssl_stream>);
        expect(_b{true} == AsioLayerableStreamSocket<ssl_stream>);
        #endif
    };

    "umbrella"_test = [] mutable {
        using tcp_socket = asio::ip::tcp::socket;

        expect(_b{true} == AsioSocket<tcp_socket>);
        expect(_b{true} == AsioStreamSocket<tcp_socket>);

        #ifdef ASIO_HAS_OPENSSL
        using ssl_stream = asio::ssl::stream<tcp_socket>;
        expect(_b{true} == AsioLayerableStreamSocket<ssl_stream>);
        #endif

        expect(_b{false} == AsioSocket<NotABufferSequence>);
        expect(_b{false} == AsioStreamSocket<BadSocketOption>);
    };

    // All tests are now registered — ut will run them and report results
}
