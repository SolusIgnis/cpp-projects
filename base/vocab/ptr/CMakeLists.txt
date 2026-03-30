# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" BASE_VOCAB_PTR_VERSION)
string(STRIP "${BASE_VOCAB_PTR_VERSION}" BASE_VOCAB_PTR_VERSION)

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Experimental "import std;" until it's a first-class feature
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
  project(base-vocab-ptr
    VERSION   ${BASE_VOCAB_PTR_VERSION}
    LANGUAGES CXX
  )
endif()

add_library(base.vocab.ptr STATIC)
add_library(base::vocab::ptr ALIAS base.vocab.ptr)

register_tooling_target(base.vocab.ptr)

# C++23 required for "import std;"
target_compile_features(base.vocab.ptr
  PUBLIC cxx_std_23
)

# Module interface units
target_sources(base.vocab.ptr
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS src
    FILES
      # Primary Module Interface Unit
      src/base.vocab.ptr.cppm
      # Partition Interface Units
      src/base.vocab.ptr-alias_ptr.cppm
      src/base.vocab.ptr-dependency_ptr.cppm
      src/base.vocab.ptr-required_ptr.cppm
)

# Implementation Units
#target_sources(base.vocab.ptr
#  PRIVATE
#    src/impl/nothing.cpp
#)

#target_link_libraries(base.vocab.ptr
#  PUBLIC
#    no::dependencies
#)

add_tests_for_module(base.vocab.ptr)
