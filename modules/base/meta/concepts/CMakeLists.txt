# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
# ===========================================================================
# //modules/base/meta/concepts/CMakeLists.txt
# ===========================================================================

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

trace_me()

add_named_module(base.meta.concepts
  LIB_TYPE STATIC
  PARTITIONS
    src/base.meta.concepts-instantiable_with.cppm
    src/base.meta.concepts-completeness.cppm
)
