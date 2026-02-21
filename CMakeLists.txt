# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors

# ===========================================================================
# Repository-Level CML
# ===========================================================================

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Experimental "import std;" until it's a first-class feature
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

project(cpp-projects
  LANGUAGES CXX
)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#For "import std"
set(CMAKE_CXX_MODULE_STD 1)

# ===========================================================================
# Check for adequate compilers supporting C++23 and "import std;".
# ===========================================================================
include(cmake/CompilerValidation.cmake)

# ===========================================================================
# Test Framework Dependencies
# ===========================================================================
include(FetchContent)

# Catch2
find_package(Catch2 3 QUIET)

if(NOT TARGET Catch2::Catch2WithMain)
  message(STATUS "Fetching Catch2 via FetchContent")
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.6.0
  )
  FetchContent_MakeAvailable(Catch2)
endif()

# GTest
find_package(GTest QUIET)

if(NOT TARGET GTest::gtest_main)
  message(STATUS "Fetching GoogleTest via FetchContent")
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
  )
  FetchContent_MakeAvailable(googletest)
endif()

# qlibs/ut
find_package(ut QUIET)

if(NOT TARGET ut::ut)
  message(STATUS "Fetching qlibs/ut via FetchContent")
  FetchContent_Declare(
    ut
    GIT_REPOSITORY https://github.com/qlibs/ut.git
    GIT_TAG v2.1.6
  )
  FetchContent_MakeAvailable(ut)
endif()

# ===========================================================================
# Test Discovery / Target Generation
# ===========================================================================
include(cmake/DiscoverTests.cmake)

# ===========================================================================
# Tooling Infrastructure (Options, Cache Variables, Registries, and Helper Functions)
# ===========================================================================
include(cmake/ToolingInfrastructure.cmake)

# ===========================================================================
# Add module group subdirectories.
# ===========================================================================

# Add the "net" module group.
add_subdirectory(net)

# ===========================================================================
# Aggregate the tooling-registered targets and their sources.
# ===========================================================================
include(cmake/ToolingSources.cmake)

# ===========================================================================
# Create tooling targets for clang-format (real and check) and clang-tidy.
# ===========================================================================
include(cmake/ToolingTargets.cmake)
