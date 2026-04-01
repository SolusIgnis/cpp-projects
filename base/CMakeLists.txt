# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

# base/CMakeLists.txt

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Add the vocab library group
add_subdirectory(vocab)

add_metamodule(base
  INTERFACE_UNIT 
    /home/runner/work/cpp-projects/cpp-projects/base/src/base.cppm
  SUBMODULES
    base::vocab
)
