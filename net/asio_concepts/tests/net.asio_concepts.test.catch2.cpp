// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file net.asio_concepts.test.catch2.cpp
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
 * @brief Unit tests for the net.asio_concepts module using Catch2.
 */

#include <catch2/catch_test_macros.hpp>

#include <asio.hpp>

import std;

import net.asio_concepts; // Code under test.

using namespace net::asio_concepts;

TEST_CASE("Asio Buffer Sequence Concepts", "[net][concepts][buffers]") {
    SECTION("Mutable Buffers") {
        CHECK(AsioMutableBufferSequence<asio::mutable_buffer>);
        CHECK(AsioMutableBufferSequence<std::array<asio::mutable_buffer, 2>>);
        CHECK_FALSE(AsioMutableBufferSequence<asio::const_buffer>);
    }

    SECTION("Const Buffers") {
        CHECK(AsioConstBufferSequence<asio::const_buffer>);
        CHECK(AsioConstBufferSequence<asio::mutable_buffer>); // Mutable satisfies Const
        CHECK(AsioConstBufferSequence<std::vector<asio::const_buffer>>);
    }
}

TEST_CASE("Asio Completion Token Concepts", "[net][concepts][tokens]") {
    SECTION("Read/Write Tokens") {
        CHECK(AsioReadToken<asio::detached_t>);
        CHECK(AsioWriteToken<asio::use_awaitable_t<>>);
        CHECK(AsioReadToken<asio::yield_context>);
    }

    SECTION("Connect/Wait Tokens") {
        CHECK(AsioConnectToken<asio::use_future_t<>>);
        CHECK(AsioWaitToken<asio::detached_t>);
    }
}

TEST_CASE("Socket Option Concepts", "[net][concepts][options]") {
    SECTION("Boolean Options") {
        CHECK(BooleanSocketOption<asio::socket_base::keep_alive>);
        CHECK(BooleanSocketOption<asio::socket_base::reuse_address>);
    }

    SECTION("Integral Options") {
        CHECK(IntegralSocketOption<asio::socket_base::receive_buffer_size>);
        CHECK(IntegralSocketOption<asio::socket_base::send_low_watermark>);
    }

    SECTION("Composite Options") {
        CHECK(CompositeSocketOption<asio::socket_base::linger>);
    }

    SECTION("Provider and Interfaces") {
        CHECK(SocketOptionProvider<asio::socket_base>);
        CHECK(SocketOptionGetter<asio::ip::tcp::socket>);
        CHECK(SocketOptionSetter<asio::ip::tcp::socket>);
    }
}

TEST_CASE("I/O Object Capabilities", "[net][concepts][io]") {
    SECTION("Executor Providers") {
        CHECK(AsioExecutorProvider<asio::ip::tcp::socket>);
        CHECK(AsioExecutorProvider<asio::steady_timer>);
    }

    SECTION("Stream Identification") {
        using tcp_sock = asio::ip::tcp::socket;
        CHECK(AsioAsyncReadStream<tcp_sock>);
        CHECK(AsioAsyncWriteStream<tcp_sock>);
        CHECK(AsioStream<tcp_sock>);
        
        // UDP does not satisfy Stream requirements
        CHECK_FALSE(AsioStream<asio::ip::udp::socket>);
    }

    SECTION("Waitable Identification") {
        CHECK(AsioAsyncTimedWaitable<asio::steady_timer>);
        CHECK(AsioAsyncActivityWaitable<asio::ip::tcp::socket>);
    }
}

TEST_CASE("Layering and Protocol Concepts", "[net][concepts][layering]") {
    SECTION("Basic Layering") {
        CHECK(LayerableObject<asio::ip::tcp::socket>);
        CHECK_FALSE(LayeredObject<asio::ip::tcp::socket>); // Base socket isn't layered
    }

#if defined(ASIO_HAS_OPENSSL)
    SECTION("SSL Layering") {
        using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;
        CHECK(LayerableObject<ssl_stream>);
        CHECK(LayeredObject<ssl_stream>);
        CHECK(AsioLayerableStreamSocket<ssl_stream>);
    }
#endif
}

TEST_CASE("Composite Socket Requirements", "[net][concepts][composition]") {
    SECTION("Full Socket Definitions") {
        CHECK(AsioSocket<asio::ip::tcp::socket>);
        CHECK(AsioSocket<asio::ip::udp::socket>);
        
        CHECK(AsioStreamSocket<asio::ip::tcp::socket>);
        CHECK_FALSE(AsioStreamSocket<asio::ip::udp::socket>);
    }
}

TEST_CASE("Lifecycle and Resource Management", "[net][concepts][lifecycle]") {
    CHECK(ClosableResource<asio::ip::tcp::socket>);
    CHECK(CancellableResource<asio::ip::tcp::socket>);
    CHECK(EndpointProvider<asio::ip::tcp::socket>);
    CHECK(NativeSocketWrapper<asio::ip::tcp::socket>);
}
