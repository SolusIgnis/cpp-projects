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

//NOLINTBEGIN: Ignore tidy for dependency includes.
#include <catch2/catch_test_macros.hpp>

#include <asio.hpp>
#include <asio/ssl.hpp>
//NOLINTEND

import std;

import net.asio_concepts; // Code under test.

using namespace net::asio_concepts; //NOLINT(google-build-using-namespace)

#define CONCEPT_CHECK(ConceptCheckAssertion)  \
    {                                         \
        static_assert(ConceptCheckAssertion); \
        CHECK(ConceptCheckAssertion);         \
    }

TEST_CASE("Asio Buffer Sequence Concepts", "[net][concepts][buffers]")
{
    SECTION("Mutable Buffers")
    {
        CONCEPT_CHECK(AsioMutableBufferSequence<asio::mutable_buffer>);
        CONCEPT_CHECK(AsioMutableBufferSequence<std::array<asio::mutable_buffer, 2>>);
        CHECK_FALSE(AsioMutableBufferSequence<asio::const_buffer>);
    }

    SECTION("Const Buffers")
    {
        CONCEPT_CHECK(AsioConstBufferSequence<asio::const_buffer>);
        CONCEPT_CHECK(AsioConstBufferSequence<asio::mutable_buffer>); // Mutable satisfies Const
        CONCEPT_CHECK(AsioConstBufferSequence<std::vector<asio::const_buffer>>);
    }
}

TEST_CASE("Asio Completion Token Concepts", "[net][concepts][tokens]")
{
    SECTION("Read/Write Tokens")
    {
        CONCEPT_CHECK(AsioReadToken<asio::detached_t>);
        CONCEPT_CHECK(AsioWriteToken<asio::use_awaitable_t<>>);
    }

    SECTION("Connect/Wait Tokens")
    {
        CONCEPT_CHECK(AsioConnectToken<asio::use_future_t<>>);
        CONCEPT_CHECK(AsioWaitToken<asio::detached_t>);
    }
}

TEST_CASE("Socket Option Concepts", "[net][concepts][options]")
{
    SECTION("Boolean Options")
    {
        CONCEPT_CHECK(BooleanSocketOption<asio::socket_base::keep_alive>);
        CONCEPT_CHECK(BooleanSocketOption<asio::socket_base::reuse_address>);
    }

    SECTION("Integral Options")
    {
        CONCEPT_CHECK(IntegralSocketOption<asio::socket_base::receive_buffer_size>);
        CONCEPT_CHECK(IntegralSocketOption<asio::socket_base::send_low_watermark>);
    }

    SECTION("Composite Options")
    {
        CONCEPT_CHECK(CompositeSocketOption<asio::socket_base::linger>);
    }

    SECTION("Provider and Interfaces")
    {
        CONCEPT_CHECK(SocketOptionProvider<asio::socket_base>);
        CONCEPT_CHECK(SocketOptionGetter<asio::ip::tcp::socket>);
        CONCEPT_CHECK(SocketOptionSetter<asio::ip::tcp::socket>);
    }
}

TEST_CASE("I/O Object Capabilities", "[net][concepts][io]")
{
    SECTION("Executor Providers")
    {
        CONCEPT_CHECK(AsioExecutorProvider<asio::ip::tcp::socket>);
        CONCEPT_CHECK(AsioExecutorProvider<asio::steady_timer>);
    }

    SECTION("Stream Identification")
    {
        using tcp_sock = asio::ip::tcp::socket;
        CONCEPT_CHECK(AsioAsyncReadStream<tcp_sock>);
        CONCEPT_CHECK(AsioAsyncWriteStream<tcp_sock>);
        CONCEPT_CHECK(AsioStream<tcp_sock>);

        // UDP does not satisfy Stream requirements
        CHECK_FALSE(AsioStream<asio::ip::udp::socket>);
    }

    SECTION("Waitable Identification")
    {
        CONCEPT_CHECK(AsioAsyncTimedWaitable<asio::steady_timer>);
        CONCEPT_CHECK(AsioAsyncActivityWaitable<asio::ip::tcp::socket>);
    }
}

TEST_CASE("Layering and Protocol Concepts", "[net][concepts][layering]")
{
    SECTION("Basic Layering")
    {
        CONCEPT_CHECK(LayerableObject<asio::ip::tcp::socket>);
        CHECK_FALSE(LayeredObject<asio::ip::tcp::socket>); // Base socket isn't layered
    }

#ifdef ASIO_HAS_OPENSSL
    SECTION("SSL Layering")
    {
        using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;
        CONCEPT_CHECK(LayerableObject<ssl_stream>);
        CONCEPT_CHECK(LayeredObject<ssl_stream>);
        CONCEPT_CHECK(AsioLayerableStreamSocket<ssl_stream>);
    }
#endif
}

TEST_CASE("Composite Socket Requirements", "[net][concepts][composition]")
{
    SECTION("Full Socket Definitions")
    {
        CONCEPT_CHECK(AsioSocket<asio::ip::tcp::socket>);
        CONCEPT_CHECK(AsioSocket<asio::ip::udp::socket>);

        CONCEPT_CHECK(AsioStreamSocket<asio::ip::tcp::socket>);
        CHECK_FALSE(AsioStreamSocket<asio::ip::udp::socket>);
    }
}

TEST_CASE("Lifecycle and Resource Management", "[net][concepts][lifecycle]")
{
    CONCEPT_CHECK(ClosableResource<asio::ip::tcp::socket>);
    CONCEPT_CHECK(CancellableResource<asio::ip::tcp::socket>);
    CONCEPT_CHECK(EndpointProvider<asio::ip::tcp::socket>);
    CONCEPT_CHECK(NativeSocketWrapper<asio::ip::tcp::socket>);
}
