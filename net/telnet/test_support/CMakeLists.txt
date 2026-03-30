# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" NET_TELNET_TEST_SUPPORT_VERSION)
string(STRIP "${NET_TELNET_TEST_SUPPORT_VERSION}" NET_TELNET_TEST_SUPPORT_VERSION)

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Experimental "import std;" until it's a first-class feature
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
  project(net-telnet-test_support
    VERSION   ${NET_TELNET_TEST_SUPPORT_VERSION}
    LANGUAGES CXX
  )
endif()

add_library(net.telnet.test_support STATIC)
add_library(net::telnet::test_support ALIAS net.telnet.test_support)

register_tooling_target(net.telnet.test_support)

# C++23 required for "import std;"
target_compile_features(net.telnet.test_support
  PUBLIC cxx_std_23
)

# Module interface units
target_sources(net.telnet.test_support
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS src
    FILES
      # Primary Module Interface Unit
      src/net.telnet.test_support.cppm
      # Partition Interface Units
      src/net.telnet.test_support-coroutine_harness.cppm
)

# Implementation Units
#target_sources(net.telnet.test_support
#  PRIVATE
#    src/impl/nothing.cpp
#)

target_link_libraries(net.telnet.test_support
  PUBLIC
    base::base
)

add_tests_for_module(net.telnet.test_support)
