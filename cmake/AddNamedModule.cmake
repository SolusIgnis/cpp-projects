# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# AddNamedModule.cmake
# ============================================================

include_guard(GLOBAL)

# ============================================================
# Let our includes see AddNamedModule.cmake was included first.
# ============================================================

set(_ADD_NAMED_MODULE_INCLUDED TRUE)

# ============================================================
# Public API
# ============================================================

function(add_named_module name)
  cmake_parse_arguments(NM_ARG
    "OBJECT;STATIC;NO_TESTS"
    "VERSION_FILE;INTERFACE_UNIT;STD"
    "BASE_DIRS;PARTITIONS;IMPLEMENTATIONS;LINK_LIBRARIES;TEST_DEPENDENCIES"
    ${ARGN}
  )

  # --- Validation ---
  if (NOT name)
    message(FATAL_ERROR "add_named_module: module name is required as first argument")
  endif()
  
  if (TARGET "${name}")
    message(FATAL_ERROR "add_named_module(${name}): target \"${name}\" already exists")
  endif()
  
  if (NOT name MATCHES "^[A-Za-z0-9_]+(\\.[A-Za-z0-9_]+)*$")
    message(FATAL_ERROR "add_named_module(${name}): invalid module name: expected <identifier>[.<identifier>]*")
  endif()
  
  if (NOT NM_ARG_INTERFACE_UNIT)
    message(FATAL_ERROR "add_named_module(${name}): INTERFACE_UNIT is required (exactly one)")
  endif()

  # Guard against accidental multiple values
  list(LENGTH NM_ARG_INTERFACE_UNIT _iface_len)
  if (NOT _iface_len EQUAL 1)
    message(FATAL_ERROR "add_named_module(${name}): INTERFACE_UNIT must contain exactly one file")
  endif()
  
  # Guard interface name to ensure it matches the target name.
  get_filename_component(_iface_name "${NM_ARG_INTERFACE_UNIT}" NAME_WE)

  if (NOT _iface_name STREQUAL "${name}")
    message(FATAL_ERROR
      "add_named_module(${name}): INTERFACE_UNIT must match module name "
      "(expected ${name}.cppm, got ${_iface_name})"
    )
  endif()
  
  # --- Default BASE_DIRS is ./src/ ---
  set(_base_dirs "src")
  if (NM_ARG_BASE_DIRS)
    set(_base_dirs ${NM_ARG_BASE_DIRS})
  endif()

  # --- Determine library type ---
  if (NM_ARG_OBJECT AND NM_ARG_STATIC)
    message(FATAL_ERROR "add_named_module(${name}): choose OBJECT or STATIC, not both")
  elseif (NM_ARG_OBJECT)
    set(_lib_type OBJECT)
  else()
    set(_lib_type STATIC)
  endif()

  # --- Target + alias ---
  add_library("${name}" "${_lib_type}")
    
  # Derive alias:
  # - base             -> base::base
  # - base.vocab       -> base::vocab
  # - base.vocab.ptr   -> base::vocab::ptr
  string(FIND "${name}" "." _dot_index)
    
  if (_dot_index EQUAL -1)
    # No dot → stutter
    set(_alias "${name}::${name}")
  else()
    # Replace all dots with ::
    string(REPLACE "." "::" _alias "${name}")
  endif()
  
  if (TARGET "${_alias}")
    message(FATAL_ERROR "add_named_module(${name}): target \"${_alias}\" already exists")
  endif()
  add_library("${_alias}" ALIAS "${name}")

  # --- Tooling ---
  if (COMMAND register_tooling_target)
    register_tooling_target("${name}")
  endif()

  # --- Language level ---
  set(cxx_std 23)
  if (NM_ARG_STD)
    set(cxx_std "${NM_ARG_STD}")
  endif()

  # Validate STD is numeric
  if (NOT cxx_std MATCHES "^[0-9]+$")
    message(FATAL_ERROR "add_named_module(${name}): STD must be a number (got '${cxx_std}')")
  endif()

  # Enforce minimum
  if (cxx_std LESS 23)
    message(FATAL_ERROR "add_named_module(${name}): STD must be >= 23 (got ${cxx_std})")
  endif()

  target_compile_features("${name}"
    PUBLIC "cxx_std_${cxx_std}"
  )

  # --- Module units ---
  set(_module_files ${NM_ARG_INTERFACE_UNIT})
  list(APPEND _module_files ${NM_ARG_PARTITIONS})

  target_sources("${name}"
    PUBLIC
      FILE_SET CXX_MODULES
      BASE_DIRS ${_base_dirs}
      FILES ${_module_files}
  )

  # --- Implementation units ---
  if (NM_ARG_IMPLEMENTATIONS)
    target_sources("${name}"
      PRIVATE
        ${NM_ARG_IMPLEMENTATIONS}
    )
  endif()

  # --- Link libraries ---
  if (NM_ARG_LINK_LIBRARIES)
    target_link_libraries("${name}"
      PUBLIC ${NM_ARG_LINK_LIBRARIES}
    )
  endif()

  # --- Tests ---
  if (NOT NM_ARG_NO_TESTS AND COMMAND add_tests_for_module)
    if (NM_ARG_TEST_DEPENDENCIES)
      add_tests_for_module("${name}"
        DEPENDENCIES ${NM_ARG_TEST_DEPENDENCIES}
      )
    else()
      add_tests_for_module("${name}")
    endif()
  endif()

endfunction()
