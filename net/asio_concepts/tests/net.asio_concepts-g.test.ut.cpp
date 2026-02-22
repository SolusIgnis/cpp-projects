// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file net.asio_concepts.test.ut.cpp
 * @date February 19, 2026
 *
 * @copyright © 2026 Jeremy Murphy and any Contributors
 * @par License: @parblock
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. @endparblock
 *
 * @brief Unit tests for the net.asio_concepts module using qlibs/ut.
 */

#include <asio.hpp>

import std;
import ut;                // Named module for qlibs/ut

import net.asio_concepts; // Code under test.

using namespace ut;
using namespace net::asio_concepts;

int main() {
    "Asio Buffer Sequence Concepts"_test = [] {
        "Mutable Buffers"_test = [] {
            expect(true_b == AsioMutableBufferSequence<asio::mutable_buffer>);
            expect(true_b == AsioMutableBufferSequence<std::array<asio::mutable_buffer, 2>>);
            expect(true_b == !AsioMutableBufferSequence<asio::const_buffer>);
        };

        "Const Buffers"_test = [] {
            expect(true_b == AsioConstBufferSequence<asio::const_buffer>);
            expect(true_b == AsioConstBufferSequence<asio::mutable_buffer>);
            expect(true_b == AsioConstBufferSequence<std::vector<asio::const_buffer>>);
        };
    };

    "Asio Completion Token Concepts"_test = [] {
        "Read/Write Tokens"_test = [] {
            expect(true_b == AsioReadToken<asio::detached_t>);
            expect(true_b == AsioWriteToken<asio::use_awaitable_t<>>);
        };

        "Connect/Wait Tokens"_test = [] {
            expect(true_b == AsioConnectToken<asio::use_future_t<>>);
            expect(true_b == AsioWaitToken<asio::detached_t>);
        };
    };

    "Socket Option Concepts"_test = [] {
        "Boolean Options"_test = [] {
            expect(true_b == BooleanSocketOption<asio::socket_base::keep_alive>);
            expect(true_b == BooleanSocketOption<asio::socket_base::reuse_address>);
        };

        "Integral Options"_test = [] {
            expect(true_b == IntegralSocketOption<asio::socket_base::receive_buffer_size>);
            expect(true_b == IntegralSocketOption<asio::socket_base::send_low_watermark>);
        };

        "Composite Options"_test = [] {
            expect(true_b == CompositeSocketOption<asio::socket_base::linger>);
        };

        "Provider and Interfaces"_test = [] {
            expect(true_b == SocketOptionProvider<asio::socket_base>);
            expect(true_b == SocketOptionGetter<asio::ip::tcp::socket>);
            expect(true_b == SocketOptionSetter<asio::ip::tcp::socket>);
        };
    };

    "I/O Object Capabilities"_test = [] {
        "Executor Providers"_test = [] {
            expect(true_b == AsioExecutorProvider<asio::ip::tcp::socket>);
            expect(true_b == AsioExecutorProvider<asio::steady_timer>);
        };

        "Stream Identification"_test = [] {
            using tcp_sock = asio::ip::tcp::socket;
            expect(true_b == AsioAsyncReadStream<tcp_sock>);
            expect(true_b == AsioAsyncWriteStream<tcp_sock>);
            expect(true_b == AsioStream<tcp_sock>);
            expect(false_b == AsioStream<asio::ip::udp::socket>);
        };

        "Waitable Identification"_test = [] {
            expect(true_b == AsioAsyncTimedWaitable<asio::steady_timer>);
            expect(true_b == AsioAsyncActivityWaitable<asio::ip::tcp::socket>);
        };
    };

    "Layering and Protocol Concepts"_test = [] {
        "Basic Layering"_test = [] {
            expect(true_b == LayerableObject<asio::ip::tcp::socket>);
            expect(true_b == !LayeredObject<asio::ip::tcp::socket>);
        };

        #if defined(ASIO_HAS_OPENSSL)
        "SSL Layering"_test = [] {
            using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;
            expect(true_b == LayerableObject<ssl_stream>);
            expect(true_b == LayeredObject<ssl_stream>);
            expect(true_b == AsioLayerableStreamSocket<ssl_stream>);
        };
        #endif
    };

    "Composite Socket Requirements"_test = [] {
        "Full Socket Definitions"_test = [] {
            expect(true_b == AsioSocket<asio::ip::tcp::socket>);
            expect(true_b == AsioSocket<asio::ip::udp::socket>);
            expect(true_b == AsioStreamSocket<asio::ip::tcp::socket>);
            expect(false_b == AsioStreamSocket<asio::ip::udp::socket>);
        };
    };

    "Lifecycle and Resource Management"_test = [] {
        expect(true_b == ClosableResource<asio::ip::tcp::socket>);
        expect(true_b == CancellableResource<asio::ip::tcp::socket>);
        expect(true_b == EndpointProvider<asio::ip::tcp::socket>);
        expect(true_b == NativeSocketWrapper<asio::ip::tcp::socket>);
    };
}
