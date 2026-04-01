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

add_named_module(base.vocab.ptr STATIC
  INTERFACE_UNIT
    src/base.vocab.ptr.cppm
  PARTITIONS
    src/base.vocab.ptr-alias_ptr.cppm
    src/base.vocab.ptr-dependency_ptr.cppm
    src/base.vocab.ptr-required_ptr.cppm
)
