# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

add_named_module(tools.test.coroutine_harness
  LIB_TYPE STATIC
  INTERFACE_UNIT
    src/tools.test.coroutine_harness.cppm
# PARTITIONS
#   src/tools.test.coroutine_harness-test_task.cppm
  LINK_LIBRARIES
    base::base
)
