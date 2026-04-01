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
# Private Helpers
# ============================================================

function(_MODULES_validate_name module_name context)
  if (NOT DEFINED context OR NOT context)
    set(context "${CMAKE_CURRENT_FUNCTION}")
  endif()
  
  if (NOT module_name MATCHES "^[A-Za-z0-9_]+(\\.[A-Za-z0-9_]+)*$")
    message(FATAL_ERROR "${context}(${module_name}): invalid module name '${module_name}'")
  endif()
endfunction()

function(_MODULES_module_to_alias out_var module_name context)
  if (NOT DEFINED context OR NOT context)
    set(context "${CMAKE_CURRENT_FUNCTION}")
  endif()
  
  _MODULES_validate_name("${module_name}" "${context}")

  string(FIND "${module_name}" "." _dot_index)

  if (_dot_index EQUAL -1)
    # No dot → stutter
    set(_alias "${module_name}::${module_name}")
  else()
    string(REPLACE "." "::" _alias "${module_name}")
  endif()

  set(${out_var} "${_alias}" PARENT_SCOPE)
endfunction()

function(_MODULES_alias_to_module out_var alias_name context)
  if (NOT DEFINED context OR NOT context)
    set(context "${CMAKE_CURRENT_FUNCTION}")
  endif()
  
  # --- Validate basic shape ---
  if (NOT alias_name MATCHES "^[A-Za-z0-9_]+(::[A-Za-z0-9_]+)+$")
    message(FATAL_ERROR "${context}(${alias_name}): invalid alias: expected <identifier>[::<identifier>]*")
  endif()

  # Split into components
  string(REPLACE "::" ";" _parts "${alias_name}")
  list(LENGTH _parts _len)

  if (_len EQUAL 2)
    # Possible stutter case: base::base
    list(GET _parts 0 _first)
    list(GET _parts 1 _second)

    if (_first STREQUAL _second)
      # Collapse stutter
      set(${out_var} "${_first}" PARENT_SCOPE)
      return()
    endif()
  endif()

  # General case: replace :: with .
  string(REPLACE "::" "." _module "${alias_name}")
  set(${out_var} "${_module}" PARENT_SCOPE)
endfunction()




# ============================================================
# Public API
# ============================================================

function(add_named_module name)
  cmake_parse_arguments(NM_ARG
    "OBJECT;STATIC;NO_TESTS"
    "INTERFACE_UNIT;STD;_CONTEXT"
    "BASE_DIRS;PARTITIONS;IMPLEMENTATIONS;IMPORTS;LINK_LIBRARIES;TEST_DEPENDENCIES"
    ${ARGN}
  )

  set(context "${NM_ARG__CONTEXT}")
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
    message(FATAL_ERROR "${context}(${name}): invalid module name: expected <identifier>[.<identifier>]*")
  endif()
  
  if (NM_ARG_INTERFACE_UNIT)
    list(LENGTH NM_ARG_INTERFACE_UNIT _iface_len)
    if (NOT _iface_len EQUAL 1)
      message(FATAL_ERROR "${context}(${name}): INTERFACE_UNIT must contain only one file")
    endif()

    set(_interface_unit "${NM_ARG_INTERFACE_UNIT}")
  else()
    # Default canonical path
    set(_interface_unit "src/${name}.cppm")
  endif()
  
  get_filename_component(_iface_path "${_interface_unit}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  
  if (NOT EXISTS "${_iface_path}")
    message(FATAL_ERROR "${context}(${name}): INTERFACE_UNIT \"${_interface_unit}\" does not exist at path \"${_iface_path}\"")
  endif()
  
  # --- Default BASE_DIRS is ./src/ ---
  set(_base_dirs "src")
  if (NM_ARG_BASE_DIRS)
    set(_base_dirs ${NM_ARG_BASE_DIRS})
  endif()

  # --- Determine library type ---
  if (NM_ARG_OBJECT AND NM_ARG_STATIC)
    message(FATAL_ERROR "${context}(${name}): choose OBJECT or STATIC, not both")
  elseif (NM_ARG_OBJECT)
    set(_lib_type OBJECT)
  else()
    set(_lib_type STATIC)
  endif()

  # --- Target + alias ---
  add_library("${name}" "${_lib_type}")
    
  _MODULES_module_to_alias(_alias "${name}")
message(STATUS "Alias: ${_alias} for ${name}")
  if (TARGET "${_alias}")
    message(FATAL_ERROR "${context}(${name}): target \"${_alias}\" already exists")
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
    message(FATAL_ERROR "${context}(${name}): STD must be a number (got '${cxx_std}')")
  endif()

  # Enforce minimum
  if (cxx_std LESS 23)
    message(FATAL_ERROR "${context}(${name}): STD must be >= 23 (got ${cxx_std})")
  endif()

  target_compile_features("${name}"
    PUBLIC "cxx_std_${cxx_std}"
  )

  # --- Module units ---
  set(_module_files ${_interface_unit})
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

  # Convert to aliases for linking
  foreach(_import_target IN LISTS NM_ARG_IMPORTS)
    _MODULES_module_to_alias(_import_target "${_import_target}")
    list(APPEND _link_targets "${_import_target}")
  endforeach() 
  
  if (NM_ARG_LINK_LIBRARIES)
    list(APPEND _link_targets ${NM_ARG_LINK_LIBRARIES})
  endif()
  
  list(REMOVE_DUPLICATES _link_targets)
  
  if (_link_targets)
    target_link_libraries("${name}"
      PUBLIC
        ${_link_targets}
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
