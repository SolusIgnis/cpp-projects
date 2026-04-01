# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

# net/CMakeLists.txt

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Add the net.asio_concepts library
add_subdirectory(asio_concepts)

# Add the net.telnet library
add_subdirectory(telnet)

add_metamodule(net STATIC
  SUBMODULES
    net.asio_concepts
    net.telnet
)
