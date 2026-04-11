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
# Add project CMake modules (./cmake/*) to CMAKE_MODULE_PATH
# ===========================================================================

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CXX_Modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Discover_Tests")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Tooling_Targets")

# ===========================================================================
# Check for adequate compilers supporting C++23 and "import std;".
# ===========================================================================
include(CompilerValidation)

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
    GIT_TAG v3.13.0
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
    GIT_TAG v1.17.0
  )
  FetchContent_MakeAvailable(googletest)
endif()

# qlibs/ut
find_package(ut QUIET)

if(NOT TARGET ut::ut)
  include(FetchUT)
  fetch_ut()
endif()

# ===========================================================================
# Test Discovery / Target Generation
# ===========================================================================
include(DiscoverTests)

# ===========================================================================
# Tooling Infrastructure (Options, Cache Variables, Registries, and Helper Functions)
# ===========================================================================
include(ToolingInfrastructure)

# ===========================================================================
# Add module utilities.
# ===========================================================================
include(AddNamedModule)
include(AddMetamodule)

# ===========================================================================
# Add module group subdirectories.
# ===========================================================================

# Add the "base" metamodule.
add_subdirectory(base)

# Add the "tools" metamodule.
add_subdirectory(tools)

# Add the "net" metamodule.
add_subdirectory(net)

# ===========================================================================
# Aggregate the tooling-registered targets and their sources.
# ===========================================================================
include(ToolingSources)

# ===========================================================================
# Create tooling targets for clang-format (real and check) and clang-tidy.
# ===========================================================================
include(ToolingTargets)
