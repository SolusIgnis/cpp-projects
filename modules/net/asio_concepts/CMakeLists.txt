# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2025 Jeremy Murphy and any Contributors
# ===========================================================================
# //modules/net/asio_concepts/CMakeLists.txt
# ===========================================================================

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

message(STATUS "Processing: ${CMAKE_CURRENT_LIST_DIR}")

# Link OpenSSL if we have it. (Tests will need it.)
find_package(OpenSSL)

add_named_module(net.asio_concepts
  LIB_TYPE OBJECT
  LINK_LIBRARIES
    $<$<BOOL:${OPENSSL_FOUND}>:OpenSSL::SSL OpenSSL::Crypto>
)
