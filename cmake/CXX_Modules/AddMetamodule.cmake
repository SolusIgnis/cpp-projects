# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# AddMetamodule.cmake
# ============================================================

include_guard(GLOBAL)

# ============================================================
# Let our includes see AddMetamodule.cmake was included first.
# ============================================================

set(_ADD_METAMODULE_INCLUDED TRUE)

# ============================================================
# Include the internal implementation helpers
# ============================================================

include(${CMAKE_CURRENT_LIST_DIR}/AddMetamodule-Internal.cmake)

# ============================================================
# Include add_named_module(...)
# ============================================================

include(${CMAKE_CURRENT_LIST_DIR}/AddNamedModule.cmake)

# ============================================================
# Public API
# ============================================================

# ============================================================
# add_metamodule(name [ARGN])
# ------------------------------------------------------------
# Define a metamodule that aggregates and re-exports a set of
# submodules (nested named modules) as a single named module
# linked as a STATIC library.
#
# Behavior:
#   - `SUBMODULES` may be provided explicitly or inferred by
#     collecting all registered modules whose parent is
#     exactly `${name}`.
#   - Submodules are passed as `IMPORTS` to `add_named_module`
#     and linked publicly from the metamodule target.
#   - If `INTERFACE_UNIT` is not specified (or is the keyword
#     `GENERATED`), an interface unit is generated in
#     `${CMAKE_CURRENT_BINARY_DIR}/generated/metamodules`
#     that `export import`s all direct submodules.
#   - If `INTERFACE_UNIT` is not specified when there is a
#     "src/" subdirectory, the call fails in order to prevent
#     silently/accidentally ignoring an authored module.
#   - Generated metamodules are excluded from tooling targets
#     since they are ephemeral, trivial, and preformatted.
#   - The directory for generated units is added to the
#     metamodule's `BASE_DIRS` automatically.
#
# Requirements:
#   - All submodule targets must be defined before calling.
#   - The metamodule passed to `name` must be the parent of
#     all `SUBMODULES` in the module tree.
#
# Forwards to:
#   add_named_module(...)
# ============================================================
function(add_metamodule name)
  cmake_parse_arguments(MM_ARG
    "NO_TESTS;NO_TOOLING"
    "INTERFACE_UNIT;STD;_CONTEXT"
    "BASE_DIRS;SUBMODULES;TEST_DEPENDENCIES"
    ${ARGN}
  )

  cxxModules_resolveContext(context "${MM_ARG__CONTEXT}")

  cxxModules_processSubmodules(_submodules "${MM_ARG_SUBMODULES}" "${name}" "${context}")

  cxxModules_resolveMetamoduleInterfaceUnit(_interface_unit _base_dirs _is_generated "${name}" "${_submodules}" "${MM_ARG_INTERFACE_UNIT}" "${context}")
  cxxModules_resolveBaseDirs(_base_dirs "${_base_dirs}" "${MM_ARG_BASE_DIRS}")

  # ============================================================
  # Argument translation layer
  # ------------------------------------------------------------
  # This section converts metamodule semantics into the list of
  # variadic arguments passed to `add_named_module`.
  #
  # Composition helper functions replace raw loops and branches.
  #
  # Generated metamodules do not require format/tidy tooling
  # integration, so they are always excluded from tooling
  # targets. Authored metamodules can also be excluded if the
  # NO_TOOLING argument flag is set. Setting the flag multiple 
  # times is idempotent.
  # ============================================================

  set(_args
    LIB_TYPE STATIC
    INTERFACE_UNIT "${_interface_unit}"
    _CONTEXT "${context}"
    IMPORTS ${_submodules}
  )

  cxxModules_appendFlagIfSet(_args NO_TESTS "${MM_ARG_NO_TESTS}")
  cxxModules_appendFlagIfSet(_args NO_TOOLING "${_is_generated}")
  cxxModules_appendFlagIfSet(_args NO_TOOLING "${MM_ARG_NO_TOOLING}")
  
  cxxModules_appendIfSet(_args STD "${MM_ARG_STD}")
  cxxModules_appendIfSet(_args BASE_DIRS "${_base_dirs}")
  cxxModules_appendIfSet(_args TEST_DEPENDENCIES "${MM_ARG_TEST_DEPENDENCIES}")

  # Forward everything to `add_named_module` for uniform validation and target creation.
  add_named_module("${name}" ${_args})  
endfunction()
