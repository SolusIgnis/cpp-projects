# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
# ===========================================================================
# //modules/net/CMakeLists.txt
# ===========================================================================

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

message(STATUS "Processing: ${CMAKE_CURRENT_LIST_DIR}")

# ===========================================================================
# Add submodule directories in dependency order.
# ===========================================================================

# Add the `net.asio_concepts` module
add_subdirectory("asio_concepts")

# Add the `net.telnet` module
# Note: dependency `net.asio_concepts` MUST be added before
add_subdirectory("telnet")

# ===========================================================================
# Add generated metamodule from submodules registered above.
# ===========================================================================

add_metamodule(net)
