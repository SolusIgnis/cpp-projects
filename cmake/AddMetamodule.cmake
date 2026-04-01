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
# Public API
# ============================================================

function(add_metamodule name)
  cmake_parse_arguments(MM_ARG
    "NO_TESTS"
    "INTERFACE_UNIT;STD;_CONTEXT"
    "SUBMODULES;TEST_DEPENDENCIES"
    ${ARGN}
  )

  set(context "${MM_ARG__CONTEXT}")
  if (NOT context)
    set(context "${CMAKE_CURRENT_FUNCTION}")
  endif()

  # --- Validation ---
  if (NOT name)
    message(FATAL_ERROR "${context}: module name is required as first argument")
  endif()

  if (TARGET "${name}")
    message(FATAL_ERROR "${context}(${name}): target \"${name}\" already exists")
  endif()

  if (NOT name MATCHES "^[A-Za-z0-9_]+(\\.[A-Za-z0-9_]+)*$")
    message(FATAL_ERROR
      "${context}(${name}): invalid module name"
    )
  endif()

  if (NOT MM_ARG_SUBMODULES)
    message(FATAL_ERROR
      "${context}(${name}): SUBMODULES is required"
    )
  endif()

  # --- Enforce metamodule invariants ---

  # 1. Metamodule must not list itself
  list(FIND MM_ARG_SUBMODULES "${name}" _self_index)
  if (NOT _self_index EQUAL -1)
    message(FATAL_ERROR "${context}(${name}): cannot include itself in SUBMODULES")
  endif()

  foreach(_sub ${MM_ARG_SUBMODULES})
    # 2. Submodules must be in namespace
    if (NOT _sub MATCHES "^${name}(\\.|$)")
      message(FATAL_ERROR "${context}(${name}): submodule '${_sub}' is not in namespace '${name}'")
    endif()
    # 3. Submodules must be valid targets
    if (NOT TARGET "${_sub}")
      message(FATAL_ERROR "${context}(${name}): submodule '${_sub}' is not a known target")
    endif()
  endforeach()

  # --- Forward to add_named_module ---

  set(_args
    STATIC
    IMPORTS ${MM_ARG_SUBMODULES}
  )

  if (MM_ARG_INTERFACE_UNIT)
    list(APPEND _args INTERFACE_UNIT ${MM_ARG_INTERFACE_UNIT})
  endif()

  if (MM_ARG_STD)
    list(APPEND _args STD ${MM_ARG_STD})
  endif()

  if (MM_ARG_NO_TESTS)
    list(APPEND _args NO_TESTS)
  endif()

  if (MM_ARG_TEST_DEPENDENCIES)
    list(APPEND _args TEST_DEPENDENCIES ${MM_ARG_TEST_DEPENDENCIES})
  endif()

  add_named_module(${name} ${_args})

endfunction()
