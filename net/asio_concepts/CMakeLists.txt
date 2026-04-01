# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2025 Jeremy Murphy and any Contributors

# net/asio_concepts/CMakeLists.txt

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" NET_ASIO_CONCEPTS_VERSION)
string(STRIP "${NET_ASIO_CONCEPTS_VERSION}" NET_ASIO_CONCEPTS_VERSION)

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Experimental "import std;" until it's a first-class feature
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
  project(net-asio-concepts
    VERSION   ${NET_ASIO_CONCEPTS_VERSION}
    LANGUAGES CXX
  )
endif()

# Link OpenSSL if we have it. (Tests will need it.)
find_package(OpenSSL)

add_named_module(net.asio_concepts OBJECT
  INTERFACE_UNIT
    src/net.asio_concepts.cppm
  LINK_LIBRARIES
    $<$<BOOL:${OPENSSL_FOUND}>:OpenSSL::SSL OpenSSL::Crypto>
)
