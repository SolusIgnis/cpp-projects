# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

# base/vocab/CMakeLists.txt

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Add the ptr library
add_subdirectory(ptr)

add_named_module(base.vocab STATIC
  IMPORTS
    base.vocab.ptr
)
