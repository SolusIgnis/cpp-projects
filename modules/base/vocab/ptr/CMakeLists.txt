# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
# ===========================================================================
# //modules/base/vocab/ptr/CMakeLists.txt
# ===========================================================================

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

trace_me()

add_named_module(base.vocab.ptr
  LIB_TYPE STATIC
  PARTITIONS
    src/base.vocab.ptr-metadata.cppm
    src/base.vocab.ptr-nullability_policies.cppm
    src/base.vocab.ptr-pointer_binding_policies.cppm
    src/base.vocab.ptr-reference_binding_policies.cppm
    src/base.vocab.ptr-traversal_policies.cppm
    src/base.vocab.ptr-core_policies.cppm
    src/base.vocab.ptr-policies.cppm
    src/base.vocab.ptr-core.cppm
    src/base.vocab.ptr-alias_ptr.cppm
    src/base.vocab.ptr-cursor_ptr.cppm
    src/base.vocab.ptr-dependency_ptr.cppm
    src/base.vocab.ptr-required_ptr.cppm
  IMPORTS
    base.meta.sequences
    base.meta.traits
    base.meta.concepts
  TEST_DEPENDENCIES
    base.meta.concepts
)
