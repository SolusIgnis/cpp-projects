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

  # --- Forward to add_named_module ---

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
  cxxModules_appendIfSet(_args TEST_DEPENDENCIES "${MM_ARG_TEST_DEPENDENCIES}")
  cxxModules_appendIfSet(_args BASE_DIRS "${_base_dirs}")

  add_named_module("${name}" ${_args})  
endfunction()
