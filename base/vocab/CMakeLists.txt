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

add_library(base.vocab STATIC)
add_library(base::vocab ALIAS base.vocab)

register_tooling_target(base.vocab)

# C++23 required for "import std;"
target_compile_features(base.vocab
  PUBLIC cxx_std_23
)

# Module interface units
target_sources(base.vocab
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS src
    FILES
      # Primary Module Interface Unit
      src/base.cppm
)

# Implementation Units
#target_sources(base.vocab
#  PRIVATE
#    src/impl/nothing.cpp
#)

# Link against all the sub-modules of the group.
target_link_libraries(base.vocab
  PUBLIC
    base::vocab::ptr
)

add_tests_for_module(base.vocab)
