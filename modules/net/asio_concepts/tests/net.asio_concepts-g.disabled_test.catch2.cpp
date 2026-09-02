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
        using mbarray = std::array<asio::mutable_buffer, 2>;
        CONCEPT_CHECK(asio_mutable_buffer_sequence<asio::mutable_buffer>);
        CONCEPT_CHECK(asio_mutable_buffer_sequence<mbarray>);
        CHECK_FALSE(asio_mutable_buffer_sequence<asio::const_buffer>);
    }

    SECTION("Const Buffers")
    {
        CONCEPT_CHECK(asio_const_buffer_sequence<asio::const_buffer>);
        CONCEPT_CHECK(asio_const_buffer_sequence<asio::mutable_buffer>); // Mutable satisfies Const
        CONCEPT_CHECK(asio_const_buffer_sequence<std::vector<asio::const_buffer>>);
    }
}

TEST_CASE("Asio Completion Token Concepts", "[net][concepts][tokens]")
{
    SECTION("Read/Write Tokens")
    {
        CONCEPT_CHECK(asio_read_token<asio::detached_t>);
        CONCEPT_CHECK(asio_write_token<asio::use_awaitable_t<>>);
    }

    SECTION("Connect/Wait Tokens")
    {
        CONCEPT_CHECK(asio_connect_token<asio::use_future_t<>>);
        CONCEPT_CHECK(asio_wait_token<asio::detached_t>);
    }
}

TEST_CASE("Socket Option Concepts", "[net][concepts][options]")
{
    SECTION("Boolean Options")
    {
        CONCEPT_CHECK(boolean_socket_option<asio::socket_base::keep_alive>);
        CONCEPT_CHECK(boolean_socket_option<asio::socket_base::reuse_address>);
    }

    SECTION("Integral Options")
    {
        CONCEPT_CHECK(integral_socket_option<asio::socket_base::receive_buffer_size>);
        CONCEPT_CHECK(integral_socket_option<asio::socket_base::send_low_watermark>);
    }

    SECTION("Composite Options")
    {
        CONCEPT_CHECK(composite_socket_option<asio::socket_base::linger>);
    }

    SECTION("Provider and Interfaces")
    {
        CONCEPT_CHECK(socket_option_provider<asio::socket_base>);
        CONCEPT_CHECK(socket_option_getter<asio::ip::tcp::socket>);
        CONCEPT_CHECK(socket_option_setter<asio::ip::tcp::socket>);
    }
}

TEST_CASE("I/O Object Capabilities", "[net][concepts][io]")
{
    SECTION("Executor Providers")
    {
        CONCEPT_CHECK(asio_executor_provider<asio::ip::tcp::socket>);
        CONCEPT_CHECK(asio_executor_provider<asio::steady_timer>);
    }

    SECTION("Stream Identification")
    {
        using tcp_sock = asio::ip::tcp::socket;
        CONCEPT_CHECK(asio_async_read_stream<tcp_sock>);
        CONCEPT_CHECK(asio_async_write_stream<tcp_sock>);
        CONCEPT_CHECK(asio_stream<tcp_sock>);

        // UDP does not satisfy Stream requirements
        CHECK_FALSE(asio_stream<asio::ip::udp::socket>);
    }

    SECTION("Waitable Identification")
    {
        CONCEPT_CHECK(asio_async_timed_waitable<asio::steady_timer>);
        CONCEPT_CHECK(asio_async_activity_waitable<asio::ip::tcp::socket>);
    }
}

TEST_CASE("Layering and Protocol Concepts", "[net][concepts][layering]")
{
    SECTION("Basic Layering")
    {
        CONCEPT_CHECK(layerable_object<asio::ip::tcp::socket>);
        CHECK_FALSE(layered_object<asio::ip::tcp::socket>); // Base socket isn't layered
    }

#ifdef ASIO_HAS_OPENSSL
    SECTION("SSL Layering")
    {
        using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;
        CONCEPT_CHECK(layerable_object<ssl_stream>);
        CONCEPT_CHECK(layered_object<ssl_stream>);
        CONCEPT_CHECK(asio_layerable_stream_socket<ssl_stream>);
    }
#endif
}

TEST_CASE("Composite Socket Requirements", "[net][concepts][composition]")
{
    SECTION("Full Socket Definitions")
    {
        CONCEPT_CHECK(asio_socket<asio::ip::tcp::socket>);
        CONCEPT_CHECK(asio_socket<asio::ip::udp::socket>);

        CONCEPT_CHECK(asio_stream_socket<asio::ip::tcp::socket>);
        CHECK_FALSE(asio_stream_socket<asio::ip::udp::socket>);
    }
}

TEST_CASE("Lifecycle and Resource Management", "[net][concepts][lifecycle]")
{
    CONCEPT_CHECK(closable_resource<asio::ip::tcp::socket>);
    CONCEPT_CHECK(cancellable_resource<asio::ip::tcp::socket>);
    CONCEPT_CHECK(endpoint_provider<asio::ip::tcp::socket>);
    CONCEPT_CHECK(native_socket_wrapper<asio::ip::tcp::socket>);
}
