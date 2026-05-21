# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
# ===========================================================================
# //modules/base/meta/sequences/CMakeLists.txt
# ===========================================================================

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

trace_me()

add_named_module(base.meta.sequences
  LIB_TYPE STATIC
  PARTITIONS
    src/base.meta.sequences-core.cppm
    src/base.meta.sequences-access.cppm
    src/base.meta.sequences-query.cppm
#    src/base.meta.sequences-select.cppm
#    src/base.meta.sequences-modify.cppm
#    src/base.meta.sequences-transform.cppm
#    src/base.meta.sequences-reduce.cppm
#    src/base.meta.sequences-invoke.cppm
)
