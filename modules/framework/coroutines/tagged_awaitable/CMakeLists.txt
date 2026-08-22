# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
# ===========================================================================
# //modules/framework/coroutines/tagged_awaitable/CMakeLists.txt
# ===========================================================================

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

trace_me()

add_named_module(framework.coroutines.tagged_awaitable
  LIB_TYPE STATIC
  TEST_DEPENDENCIES
    tools.test.coroutine_harness
)
