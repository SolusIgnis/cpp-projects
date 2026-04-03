# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# CXXModules-Internal.cmake

if(NOT _CXXMODULES_INCLUDED
   AND
   NOT _ADD_METAMODULE_INCLUDED
   AND
   NOT _ADD_NAMED_MODULE_INCLUDED)
  message(FATAL_ERROR "CXXModules-Internal.cmake is internal and must not be included directly.")
endif()

include_guard(GLOBAL)

# ============================================================
# Make sure we have our tooling registration function.
# ============================================================

include(ToolingInfrastructure)

# ============================================================
# Private Helpers
# ============================================================

macro("cxxModules.resolve_context" out_var provided_context)
  if (provided_context)
    set(${out_var} "${provided_context}")
  else()
    set(${out_var} "${CMAKE_CURRENT_FUNCTION}")
  endif()
endmacro()

function("cxxModules.validate_name" module_name context)
  cxxModules.resolve_context(context "${context}")
  
  if (NOT module_name MATCHES "^[A-Za-z0-9_]+(\\.[A-Za-z0-9_]+)*$")
    message(FATAL_ERROR "${context}(${module_name}): invalid module name '${module_name}'")
  endif()
endfunction()




function("cxxModules.split_module_name" out_list module_name context)
  cxxModules.resolve_context(context "${context}")
  
  cxxModules.validate_name("${module_name}" "${context}")

  string(REPLACE "." ";" _parts "${module_name}")
  set(${out_list} ${_parts} PARENT_SCOPE)
endfunction()

function("cxxModules.parent_module" out_var module_name context)
  cxxModules.resolve_context(context "${context}")

  cxxModules.split_module_name(_parts "${module_name}" "${context}")
  
  list(LENGTH _parts _len)

  if (_len LESS 2)
    set(${out_var} "" PARENT_SCOPE)
    return()
  endif()

  list(POP_BACK _parts)
  string(JOIN "." _parent ${_parts})
  set(${out_var} "${_parent}" PARENT_SCOPE)
endfunction()

function("cxxModules.core_name" out_var module_name context)
  cxxModules.resolve_context(context "${context}")

  cxxModules.split_module_name(_parts "${module_name}" "${context}")
  list(GET _parts -1 _leaf)
  set(${out_var} "${_leaf}" PARENT_SCOPE)
endfunction()



function("cxxModules.module_to_alias" out_var module_name context)
  cxxModules.resolve_context(context "${context}")
  
  cxxModules.validate_name("${module_name}" "${context}")

  string(FIND "${module_name}" "." _dot_index)

  if (_dot_index EQUAL -1)
    # No dot → stutter
    set(_alias "${module_name}::${module_name}")
  else()
    string(REPLACE "." "::" _alias "${module_name}")
  endif()

  set(${out_var} "${_alias}" PARENT_SCOPE)
endfunction()

function("cxxModules.alias_to_module" out_var alias_name context)
  cxxModules.resolve_context(context "${context}")
  
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


function("cxxModules.append_if_set" list_var key value)
  if (value)
    list(APPEND ${list_var} ${key} ${value})
    set(${list_var} "${${list_var}}" PARENT_SCOPE)
  endif()
endfunction()
