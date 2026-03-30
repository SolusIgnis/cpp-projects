# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" TOOLS__TEST__COROUTINE_HARNESS__VERSION)
string(STRIP "${TOOLS__TEST__COROUTINE_HARNESS__VERSION}" TOOLS__TEST__COROUTINE_HARNESS__VERSION)

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Experimental "import std;" until it's a first-class feature
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
  project(tools-test-coroutine_harness 
    VERSION   ${TOOLS__TEST__COROUTINE_HARNESS__VERSION}
    LANGUAGES CXX
  )
endif()

add_library(tools.test.coroutine_harness STATIC)
add_library(tools::test::coroutine_harness ALIAS tools.test.coroutine_harness)

register_tooling_target(tools.test.coroutine_harness)

# C++23 required for "import std;"
target_compile_features(tools.test.coroutine_harness
  PUBLIC cxx_std_23
)

# Module interface units
target_sources(tools.test.coroutine_harness
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS src
    FILES
      # Primary Module Interface Unit
      src/tools.test.coroutine_harness.cppm
      # Partition Interface Units
      #src/tools.test.coroutine_harness-test_task.cppm
)

# Implementation Units
#target_sources(tools.test.coroutine_harness
#  PRIVATE
#    src/impl/nothing.cpp
#)

target_link_libraries(tools.test.coroutine_harness
  PUBLIC
    base::base
)

add_tests_for_module(tools.test.coroutine_harness)
