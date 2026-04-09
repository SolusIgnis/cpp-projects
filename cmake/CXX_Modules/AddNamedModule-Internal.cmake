# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# AddNamedModule-Internal.cmake
# ============================================================

if(NOT _ADD_NAMED_MODULE_INCLUDED)
  message(FATAL_ERROR "AddNamedModule-Internal.cmake is internal and must not be included directly.")
endif()

include_guard(GLOBAL)

# ============================================================
# Let our includes see AddNamedModule-Internal.cmake was included first.
# ============================================================

set(_ADD_NAMED_MODULE_INTERNAL_INCLUDED TRUE)

# ============================================================
# Include the internal implementation helpers
# ============================================================

include(${CMAKE_CURRENT_LIST_DIR}/CXXModules-Internal.cmake)

# ============================================================
# Private Helpers
# ============================================================

function(cxxModules_validateInterfaceUnit out_var interface_unit module_name context)
  cxxModules_resolveContext(context "${context}")

  if (interface_unit)
    list(LENGTH interface_unit _iface_len)
    if (NOT _iface_len EQUAL 1)
      message(FATAL_ERROR "${context}(${module_name}): INTERFACE_UNIT must contain only one file")
    endif()

    set(_iface_unit "${interface_unit}")
  else()
    # Default canonical path
    set(_iface_unit "src/${module_name}.cppm")
  endif()
  
  if(IS_ABSOLUTE "${_iface_unit}")
    set(_iface_path "${_iface_unit}")
  else()
    get_filename_component(_iface_path "${_iface_unit}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()
  
  if (NOT EXISTS "${_iface_path}")
    message(FATAL_ERROR "${context}(${module_name}): INTERFACE_UNIT '${_iface_unit}' does not exist at path '${_iface_path}'")
  endif()
  
  set(${out_var} "${_iface_unit}" PARENT_SCOPE)
endfunction()

function(cxxModules_validateStd cxx_std context)
  cxxModules_resolveContext(context "${context}")

  # Validate STD is numeric
  if (NOT cxx_std MATCHES "^[0-9]+$")
    message(FATAL_ERROR "${context}: STD must be a number (got '${cxx_std}')")
  endif()

  # Enforce minimum
  if (cxx_std LESS 23)
    message(FATAL_ERROR "${context}: STD must be >= 23 (got ${cxx_std})")
  endif()
endfunction()

function(cxxModules_collectLinkTargets out_var imports libraries context)
  cxxModules_resolveContext(context "${context}")
  
  set(_link_targets)
  # Convert to aliases for linking
  foreach(_import_target IN LISTS imports)
    cxxModules_moduleToAlias(_import_target "${_import_target}" "${context}")
    list(APPEND _link_targets "${_import_target}")
  endforeach() 
  
  if (libraries)
    list(APPEND _link_targets ${libraries})
  endif()
  
  list(REMOVE_DUPLICATES _link_targets)
  
  set(${out_var} "${_link_targets}" PARENT_SCOPE)
endfunction()

function(cxxModules_validateLibType out_var lib_type module_name context)
  cxxModules_resolveContext(context "${context}")
  string(TOUPPER "${lib_type}" lib_type)
  
  # --- Determine library type ---
  if ("${lib_type}" STREQUAL "STATIC")
    set(${out_var} STATIC PARENT_SCOPE)
  elseif("${lib_type}" STREQUAL "OBJECT")
    set(${out_var} OBJECT PARENT_SCOPE)
  elseif("${lib_type}" STREQUAL "SHARED")
    message(WARNING "${context}(${module_name}): SHARED library type potentially dangerous for named modules.")
    set(${out_var} SHARED PARENT_SCOPE)
  else()
    message(FATAL_ERROR "${context}(${module_name}): LIB_TYPE should be either 'OBJECT' or 'STATIC'.")
  endif()
endfunction()

function(cxxModules_registerModule module_name context)
  cxxModules_resolveContext(context "${context}")

  set(_registered_modules "${CXXMODULES_REGISTERED_MODULES}")
  # --- Prevent duplicate registration ---
  list(FIND _registered_modules "${module_name}" _exists)
  if (NOT _exists EQUAL -1)
    message(FATAL_ERROR "${context}(${module_name}): module already registered")
  endif()

  # --- Append to global registry ---
  list(APPEND _registered_modules "${module_name}")
  set(CXXMODULES_REGISTERED_MODULES "${_registered_modules}" CACHE INTERNAL "" FORCE)
endfunction()
