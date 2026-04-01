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
    "INTERFACE_UNIT;STD"
    "SUBMODULES;TEST_DEPENDENCIES"
    ${ARGN}
  )

  # --- Validation ---
  if (NOT name)
    message(FATAL_ERROR "add_metamodule: module name is required as first argument")
  endif()

  if (TARGET "${name}")
    message(FATAL_ERROR "add_metamodule(${name}): target \"${name}\" already exists")
  endif()

  if (NOT name MATCHES "^[A-Za-z0-9_]+(\\.[A-Za-z0-9_]+)*$")
    message(FATAL_ERROR
      "add_metamodule(${name}): invalid module name"
    )
  endif()

  if (NOT MM_ARG_INTERFACE_UNIT)
    message(FATAL_ERROR
      "add_metamodule(${name}): INTERFACE_UNIT is required"
    )
  endif()

  list(LENGTH MM_ARG_INTERFACE_UNIT _iface_len)
  if (NOT _iface_len EQUAL 1)
    message(FATAL_ERROR
      "add_metamodule(${name}): INTERFACE_UNIT must contain exactly one file"
    )
  endif()

  if (NOT MM_ARG_SUBMODULES)
    message(FATAL_ERROR
      "add_metamodule(${name}): SUBMODULES is required"
    )
  endif()

  # --- Enforce metamodule invariants ---

  # 1. Metamodule must not list itself
  list(FIND MM_ARG_SUBMODULES "${name}" _self_index)
  if (NOT _self_index EQUAL -1)
    message(FATAL_ERROR "add_metamodule(${name}): cannot include itself in SUBMODULES")
  endif()

  # 2. Submodules must be in namespace
  foreach(_sub ${MM_ARG_SUBMODULES})
    if (NOT _sub MATCHES "^${name}::")
      message(FATAL_ERROR "add_metamodule(${name}): submodule '${_sub}' is not in namespace '${name}'")
    endif()
  endforeach()

  # --- Forward to add_named_module ---

  set(_args
    STATIC
    INTERFACE_UNIT ${MM_ARG_INTERFACE_UNIT}
    LINK_LIBRARIES ${MM_ARG_SUBMODULES}
  )

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
