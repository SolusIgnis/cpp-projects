# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors

# net/CMakeLists.txt

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Experimental "import std;" until it's a first-class feature
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
  project(net-bundle
    LANGUAGES CXX
  )
endif()

# Add the net.asio_concepts library
add_subdirectory(asio_concepts)

# Add the net.telnet library
add_subdirectory(telnet)

add_library(net STATIC)
add_library(net::net ALIAS net)

register_tooling_target(net)

# C++23 required for "import std;"
target_compile_features(net
  PUBLIC cxx_std_23
)

# Module interface units
target_sources(net
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS src
    FILES
      # Primary Module Interface Unit
      src/net.cppm
)

# Implementation Units
#target_sources(net
#  PRIVATE
#    src/impl/nothing.cpp
#)

# Link against all the sub-modules of the group.
target_link_libraries(net
  PUBLIC
    net::asio_concepts
    net::telnet
)

add_tests_for_module(net)
