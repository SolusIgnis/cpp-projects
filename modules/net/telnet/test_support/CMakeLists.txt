# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" NET_TELNET_TEST_SUPPORT_VERSION)
string(STRIP "${NET_TELNET_TEST_SUPPORT_VERSION}" NET_TELNET_TEST_SUPPORT_VERSION)

add_named_module(net.telnet.test_support
  LIB_TYPE STATIC
  PARTITIONS
    src/net.telnet.test_support-coroutine_harness.cppm
  IMPORTS
    base
)
