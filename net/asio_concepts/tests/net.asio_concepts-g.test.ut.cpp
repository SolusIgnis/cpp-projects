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
            expect(AsioMutableBufferSequence<asio::mutable_buffer>);
            expect(AsioMutableBufferSequence<std::array<asio::mutable_buffer, 2>>);
            expect(!AsioMutableBufferSequence<asio::const_buffer>);
        };

        "Const Buffers"_test = [] {
            expect(AsioConstBufferSequence<asio::const_buffer>);
            expect(AsioConstBufferSequence<asio::mutable_buffer>);
            expect(AsioConstBufferSequence<std::vector<asio::const_buffer>>);
        };
    };

    "Asio Completion Token Concepts"_test = [] {
        "Read/Write Tokens"_test = [] {
            expect(AsioReadToken<asio::detached_t>);
            expect(AsioWriteToken<asio::use_awaitable_t<>>);
        };

        "Connect/Wait Tokens"_test = [] {
            expect(AsioConnectToken<asio::use_future_t<>>);
            expect(AsioWaitToken<asio::detached_t>);
        };
    };

    "Socket Option Concepts"_test = [] {
        "Boolean Options"_test = [] {
            expect(BooleanSocketOption<asio::socket_base::keep_alive>);
            expect(BooleanSocketOption<asio::socket_base::reuse_address>);
        };

        "Integral Options"_test = [] {
            expect(IntegralSocketOption<asio::socket_base::receive_buffer_size>);
            expect(IntegralSocketOption<asio::socket_base::send_low_watermark>);
        };

        "Composite Options"_test = [] {
            expect(CompositeSocketOption<asio::socket_base::linger>);
        };

        "Provider and Interfaces"_test = [] {
            expect(SocketOptionProvider<asio::socket_base>);
            expect(SocketOptionGetter<asio::ip::tcp::socket>);
            expect(SocketOptionSetter<asio::ip::tcp::socket>);
        };
    };

    "I/O Object Capabilities"_test = [] {
        "Executor Providers"_test = [] {
            expect(AsioExecutorProvider<asio::ip::tcp::socket>);
            expect(AsioExecutorProvider<asio::steady_timer>);
        };

        "Stream Identification"_test = [] {
            using tcp_sock = asio::ip::tcp::socket;
            expect(AsioAsyncReadStream<tcp_sock>);
            expect(AsioAsyncWriteStream<tcp_sock>);
            expect(AsioStream<tcp_sock>);
            expect(!AsioStream<asio::ip::udp::socket>);
        };

        "Waitable Identification"_test = [] {
            expect(AsioAsyncTimedWaitable<asio::steady_timer>);
            expect(AsioAsyncActivityWaitable<asio::ip::tcp::socket>);
        };
    };

    "Layering and Protocol Concepts"_test = [] {
        "Basic Layering"_test = [] {
            expect(LayerableObject<asio::ip::tcp::socket>);
            expect(!LayeredObject<asio::ip::tcp::socket>);
        };

        #if defined(ASIO_HAS_OPENSSL)
        "SSL Layering"_test = [] {
            using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;
            expect(LayerableObject<ssl_stream>);
            expect(LayeredObject<ssl_stream>);
            expect(AsioLayerableStreamSocket<ssl_stream>);
        };
        #endif
    };

    "Composite Socket Requirements"_test = [] {
        "Full Socket Definitions"_test = [] {
            expect(AsioSocket<asio::ip::tcp::socket>);
            expect(AsioSocket<asio::ip::udp::socket>);
            expect(AsioStreamSocket<asio::ip::tcp::socket>);
            expect(!AsioStreamSocket<asio::ip::udp::socket>);
        };
    };

    "Lifecycle and Resource Management"_test = [] {
        expect(ClosableResource<asio::ip::tcp::socket>);
        expect(CancellableResource<asio::ip::tcp::socket>);
        expect(EndpointProvider<asio::ip::tcp::socket>);
        expect(NativeSocketWrapper<asio::ip::tcp::socket>);
    };
}
