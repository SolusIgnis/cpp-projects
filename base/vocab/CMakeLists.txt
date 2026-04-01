# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

# base/vocab/CMakeLists.txt

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Experimental "import std;" until it's a first-class feature
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
  project(base-vocab-bundle
    LANGUAGES CXX
  )
endif()

# Add the ptr library
add_subdirectory(ptr)

add_named_module(base.vocab STATIC
  IMPORTS
    base.vocab.ptr
)
