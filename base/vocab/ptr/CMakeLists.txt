# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" BASE_VOCAB_PTR_VERSION)
string(STRIP "${BASE_VOCAB_PTR_VERSION}" BASE_VOCAB_PTR_VERSION)

add_named_module(base.vocab.ptr
  LIB_TYPE STATIC
  PARTITIONS
    src/base.vocab.ptr-alias_ptr.cppm
    src/base.vocab.ptr-dependency_ptr.cppm
    src/base.vocab.ptr-required_ptr.cppm
)
