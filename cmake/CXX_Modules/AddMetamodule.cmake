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
    "NO_TESTS"
    "INTERFACE_UNIT;STD;_CONTEXT"
    "SUBMODULES;TEST_DEPENDENCIES"
    ${ARGN}
  )

  cxxModules_resolveContext(context "${MM_ARG__CONTEXT}")

  if (MM_ARG_SUBMODULES)
    set(_submodules "${MM_ARG_SUBMODULES}")
  else()
    cxxModules_collectRegisteredSubmodules(_submodules "${name}" "${context}")
  endif()

  list(REMOVE_DUPLICATES _submodules)
  # SUBMODULES are treated as an unordered set; ordering is normalized
  list(SORT _submodules)

  # --- Enforce metamodule invariants ---

  if (NOT _submodules)
    message(FATAL_ERROR "${context}(${name}): no submodules found for '${name}'. Ensure submodules are registered before adding parent metamodules or specify SUBMODULES explicitly.")
  endif()

  # 1. Metamodule must not list itself
  list(FIND _submodules "${name}" _self_index)
  if (NOT _self_index EQUAL -1)
    message(FATAL_ERROR "${context}(${name}): ${name} cannot include itself in SUBMODULES")
  endif()

  foreach(_sub IN LISTS _submodules)
    # 2. Metamodule must be parent of submodules
    cxxModules_parentModule(_parent "${_sub}" "${context}")
    if (NOT _parent STREQUAL name)
      message(FATAL_ERROR "${context}(${name}): metamodule '${name}' is not the parent of submodule '${_sub}'")
    endif()
    # 3. Submodules must be valid targets
    if (NOT TARGET "${_sub}")
      message(FATAL_ERROR "${context}(${name}): submodule '${_sub}' is not a known target")
    endif()
  endforeach()

  # --- Forward to add_named_module ---

  set(_args
    LIB_TYPE STATIC
    _CONTEXT "${context}"
    IMPORTS ${_submodules}
  )

  cxxModules_appendFlagIfSet(_args NO_TESTS "${MM_ARG_NO_TESTS}")
  
  cxxModules_appendIfSet(_args INTERFACE_UNIT "${MM_ARG_INTERFACE_UNIT}")
  cxxModules_appendIfSet(_args STD "${MM_ARG_STD}")
  cxxModules_appendIfSet(_args TEST_DEPENDENCIES "${MM_ARG_TEST_DEPENDENCIES}")

  add_named_module("${name}" ${_args})

endfunction()
