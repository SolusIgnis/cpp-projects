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

using namespace boost::ut;  // or using namespace ut; depending on your ut version
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

// Helper to test concepts that require member types / nested typedefs
template<typename T>
concept HasMessageFlags = requires { typename T::message_flags; };

// ─────────────────────────────────────────────────────────────────────────────
// Main test suite
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    tag("buffers") / [] {
        expect(AsioMutableBufferSequence<asio::mutable_buffer>);
        expect(AsioMutableBufferSequence<std::array<asio::mutable_buffer, 4>>);
        expect(AsioMutableBufferSequence<std::vector<asio::mutable_buffer>>);

        expect(AsioConstBufferSequence<asio::const_buffer>);
        expect(AsioConstBufferSequence<std::span<const asio::const_buffer>>);

        expect(not AsioMutableBufferSequence<NotABufferSequence>);
        expect(not AsioConstBufferSequence<NotABufferSequence>);
    };

    tag("tokens") / [] {
        expect(AsioReadToken<asio::use_awaitable>);
        expect(AsioReadToken<asio::detached>);
        expect(AsioReadToken<asio::use_future>);

        expect(AsioWriteToken<asio::use_awaitable>);
        expect(AsioConnectToken<asio::detached>);

        expect(not AsioReadToken<NotACompletionToken>);
    };

    tag("socket_options") / [] {
        using broadcast   = asio::socket_base::broadcast;
        using linger      = asio::socket_base::linger;
        using recv_buf    = asio::socket_base::receive_buffer_size;
        using join_group  = asio::ip::multicast::join_group;

        expect(BooleanSocketOption<broadcast>);
        expect(CompositeSocketOption<linger>);
        expect(IntegralSocketOption<recv_buf>);
        expect(AsioAddressibleSocketOption<join_group>);

        expect(SocketOption<broadcast>);
        expect(SocketOption<linger>);

        expect(SocketOptionProvider<asio::socket_base>);

        using tcp_socket = asio::ip::tcp::socket;
        expect(SocketOptionGetter<tcp_socket>);
        expect(SocketOptionSetter<tcp_socket>);

        expect(not BooleanSocketOption<BadSocketOption>);
    };

    tag("io_flags_executor") / [] {
        using tcp_socket = asio::ip::tcp::socket;

        expect(IOController<tcp_socket>);
        expect(MessageFlagProvider<tcp_socket>);
        expect(AsioExecutorProvider<tcp_socket>);
        expect(AsioExecutorAssociated<tcp_socket>);

        expect(not AsioExecutorAssociated<NoExecutorType>);
    };

    tag("streams") / [] {
        using tcp_socket = asio::ip::tcp::socket;

        expect(AsioAsyncReadStream<tcp_socket>);
        expect(AsioSyncReadStream<tcp_socket>);
        expect(AsioAsyncWriteStream<tcp_socket>);
        expect(AsioSyncWriteStream<tcp_socket>);

        expect(AsioStream<tcp_socket>);

        expect(not AsioAsyncReadStream<NoExecutorType>);
    };

    tag("waitables") / [] {
        using timer      = asio::steady_timer;
        using tcp_socket = asio::ip::tcp::socket;

        expect(AsioAsyncTimedWaitable<timer>);
        expect(AsioSyncTimedWaitable<timer>);

        expect(AsioAsyncActivityWaitable<tcp_socket>);
        expect(AsioSyncActivityWaitable<tcp_socket>);

        expect(not AsioAsyncTimedWaitable<FakeWaitableNoWait>);
    };

    tag("transmission") / [] {
        using tcp_socket = asio::ip::tcp::socket;

        expect(AsioAsyncSender<tcp_socket>);
        expect(AsioSyncSender<tcp_socket>);
        expect(AsioAsyncReceiver<tcp_socket>);
        expect(AsioSyncReceiver<tcp_socket>);

        expect(HasAtMark<tcp_socket>);
        expect(HasAvailable<tcp_socket>);
    };

    tag("lifecycle") / [] {
        using tcp_socket = asio::ip::tcp::socket;

        expect(CancellableResource<tcp_socket>);
        expect(ClosableResource<tcp_socket>);
        expect(EndpointProvider<tcp_socket>);
    };

    tag("connection") / [] {
        using tcp_socket = asio::ip::tcp::socket;

        expect(AsioAsyncConnectable<tcp_socket>);
        expect(AsioSyncConnectable<tcp_socket>);
        expect(NativeSocketWrapper<tcp_socket>);
    };

    tag("layering") / [] {
        using tcp_socket = asio::ip::tcp::socket;
        using ssl_stream = asio::ssl::stream<tcp_socket>;

        expect(LayerableObject<tcp_socket>);
        expect(LayerableObject<ssl_stream>);

        expect(LayeredObject<ssl_stream>);

        expect(AsioLayerableSocket<ssl_stream>);
        expect(AsioLayerableStreamSocket<ssl_stream>);
    };

    tag("umbrella") / [] {
        using tcp_socket = asio::ip::tcp::socket;
        using ssl_stream = asio::ssl::stream<tcp_socket>;

        expect(AsioSocket<tcp_socket>);
        expect(AsioStreamSocket<tcp_socket>);

        expect(AsioLayerableStreamSocket<ssl_stream>);

        expect(not AsioSocket<NotABufferSequence>);
        expect(not AsioStreamSocket<BadSocketOption>);
    };

    tag("signatures") / [] {
        using Sig = void(std::error_code, std::size_t);

        expect(std::same_as<asio_read_completion_signature, Sig>);
        expect(std::same_as<asio_write_completion_signature, Sig>);
        expect(std::same_as<asio_wait_completion_signature, void(std::error_code)>);
    };

    // All tests are now registered — ut will run them and report results
}
