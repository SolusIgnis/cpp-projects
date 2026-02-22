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

//NOLINTBEGIN: Ignore tidy for dependency includes.
#include <asio.hpp>
#include <asio/ssl.hpp>
//NOLINTEND

import std;
import ut;                // Named module for qlibs/ut

import net.asio_concepts; // Code under test.

using namespace ut; //NOLINT(google-build-using-namespace)
using namespace net::asio_concepts; //NOLINT(google-build-using-namespace)

int main() {
    "Asio Buffer Sequence Concepts"_test = [] mutable {
        "Mutable Buffers"_test = [] mutable {
            expect(_b{true} == AsioMutableBufferSequence<asio::mutable_buffer>);
            expect(_b{true} == AsioMutableBufferSequence<std::array<asio::mutable_buffer, 2>>);
            expect(_b{false} == AsioMutableBufferSequence<asio::const_buffer>);
        };

        "Const Buffers"_test = [] mutable {
            expect(_b{true} == AsioConstBufferSequence<asio::const_buffer>);
            expect(_b{true} == AsioConstBufferSequence<asio::mutable_buffer>);
            expect(_b{true} == AsioConstBufferSequence<std::vector<asio::const_buffer>>);
        };
    };

    "Asio Completion Token Concepts"_test = [] mutable {
        "Read/Write Tokens"_test = [] mutable {
            expect(_b{true} == AsioReadToken<asio::detached_t>);
            expect(_b{true} == AsioWriteToken<asio::use_awaitable_t<>>);
        };

        "Connect/Wait Tokens"_test = [] mutable {
            expect(_b{true} == AsioConnectToken<asio::use_future_t<>>);
            expect(_b{true} == AsioWaitToken<asio::detached_t>);
        };
    };

    "Socket Option Concepts"_test = [] mutable {
        "Boolean Options"_test = [] mutable {
            expect(_b{true} == BooleanSocketOption<asio::socket_base::keep_alive>);
            expect(_b{true} == BooleanSocketOption<asio::socket_base::reuse_address>);
        };

        "Integral Options"_test = [] mutable {
            expect(_b{true} == IntegralSocketOption<asio::socket_base::receive_buffer_size>);
            expect(_b{true} == IntegralSocketOption<asio::socket_base::send_low_watermark>);
        };

        "Composite Options"_test = [] mutable {
            expect(_b{true} == CompositeSocketOption<asio::socket_base::linger>);
        };

        "Provider and Interfaces"_test = [] mutable {
            expect(_b{true} == SocketOptionProvider<asio::socket_base>);
            expect(_b{true} == SocketOptionGetter<asio::ip::tcp::socket>);
            expect(_b{true} == SocketOptionSetter<asio::ip::tcp::socket>);
        };
    };

    "I/O Object Capabilities"_test = [] mutable {
        "Executor Providers"_test = [] mutable {
            expect(_b{true} == AsioExecutorProvider<asio::ip::tcp::socket>);
            expect(_b{true} == AsioExecutorProvider<asio::steady_timer>);
        };

        "Stream Identification"_test = [] mutable {
            using tcp_sock = asio::ip::tcp::socket;
            expect(_b{true} == AsioAsyncReadStream<tcp_sock>);
            expect(_b{true} == AsioAsyncWriteStream<tcp_sock>);
            expect(_b{true} == AsioStream<tcp_sock>);
            expect(_b{false} == AsioStream<asio::ip::udp::socket>);
        };

        "Waitable Identification"_test = [] mutable {
            expect(_b{true} == AsioAsyncTimedWaitable<asio::steady_timer>);
            expect(_b{true} == AsioAsyncActivityWaitable<asio::ip::tcp::socket>);
        };
    };

    "Layering and Protocol Concepts"_test = [] mutable {
        "Basic Layering"_test = [] mutable {
            expect(_b{true} == LayerableObject<asio::ip::tcp::socket>);
            expect(_b{false} == LayeredObject<asio::ip::tcp::socket>);
        };

        #ifdef ASIO_HAS_OPENSSL
        "SSL Layering"_test = [] mutable {
            using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;
            expect(_b{true} == LayerableObject<ssl_stream>);
            expect(_b{true} == LayeredObject<ssl_stream>);
            expect(_b{true} == AsioLayerableStreamSocket<ssl_stream>);
        };
        #endif
    };

    "Composite Socket Requirements"_test = [] mutable {
        "Full Socket Definitions"_test = [] mutable {
            expect(_b{true} == AsioSocket<asio::ip::tcp::socket>);
            expect(_b{true} == AsioSocket<asio::ip::udp::socket>);
            expect(_b{true} == AsioStreamSocket<asio::ip::tcp::socket>);
            expect(_b{false} == AsioStreamSocket<asio::ip::udp::socket>);
        };
    };

    "Lifecycle and Resource Management"_test = [] mutable {
        expect(_b{true} == ClosableResource<asio::ip::tcp::socket>);
        expect(_b{true} == CancellableResource<asio::ip::tcp::socket>);
        expect(_b{true} == EndpointProvider<asio::ip::tcp::socket>);
        expect(_b{true} == NativeSocketWrapper<asio::ip::tcp::socket>);
    };
}
