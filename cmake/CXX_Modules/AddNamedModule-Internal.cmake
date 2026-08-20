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

# =================================================================
# Module Registry + Linking Resolution Layer
# =================================================================

# ============================================================
# cxxModules_validateInterfaceUnit(interface_unit module_name context)
# ------------------------------------------------------------
# Validate that `interface_unit` is a single file and that it
# exists in the filesystem.
# ============================================================
function(cxxModules_validateInterfaceUnit interface_unit module_name context)
  list(LENGTH interface_unit _iface_len)
  if (NOT _iface_len EQUAL 1)
    message(FATAL_ERROR "${context}(${module_name}): INTERFACE_UNIT must contain only one file")
  endif()
  
  if(IS_ABSOLUTE "${interface_unit}")
    set(_iface_path "${interface_unit}")
  else()
    get_filename_component(_iface_path "${interface_unit}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()
  
  if (NOT EXISTS "${_iface_path}")
    message(FATAL_ERROR "${context}(${module_name}): INTERFACE_UNIT '${interface_unit}' does not exist at path '${_iface_path}'")
  endif()
endfunction()

# ============================================================
# cxxModules_resolveInterfaceUnit(out_var interface_unit module_name context)
# ------------------------------------------------------------
# Resolve and validate the identity of a module's primary
# module interface unit, falling back to a canonical default
# (src/<module_name>.cppm) when no path argument is passed.
# ============================================================
function(cxxModules_resolveInterfaceUnit out_var interface_unit module_name context)
  cxxModules_resolveContext(context "${context}")

  cxxModules_setWithDefault(_iface_unit "${interface_unit}" "src/${module_name}.cppm")
  cxxModules_validateInterfaceUnit("${_iface_unit}" "${module_name}" "${context}")

  set(${out_var} "${_iface_unit}" PARENT_SCOPE)
endfunction()

# ============================================================
# cxxModules_validateStd(cxx_std context)
# ------------------------------------------------------------
# Validate that the requested C++ standard is supported.
# Parameter `cxx_std` must be a number and at least 23.
# ============================================================
function(cxxModules_validateStd cxx_std context)
  cxxModules_resolveContext(context "${context}")

  # Validate STD is numeric
  if (NOT cxx_std MATCHES "^[0-9]+$")
    message(FATAL_ERROR "${context}: STD must be a number (got '${cxx_std}')")
  endif()

  # Enforce minimum (C++23 is required for `import std`)⁰
  if (cxx_std LESS 23)
    message(FATAL_ERROR "${context}: STD must be >= 23 (got ${cxx_std})")
  endif()
endfunction()

# ============================================================
# cxxModules_collectLinkTargets(out_var imports libraries context)
# ------------------------------------------------------------
# Aggregate the set of link targets for a module by resolving
# imported modules into library targets and appending any
# additional library dependencies. 
#
# Output a deduplicated list of linkable library targets.
# ============================================================
function(cxxModules_collectLinkTargets out_var imports libraries context)
  cxxModules_resolveContext(context "${context}")
  
  set(_link_targets)
  # Convert to aliases for linking
  foreach(_import_target IN LISTS imports)
    cxxModules_moduleToAlias(_import_target "${_import_target}" "${context}")
    list(APPEND _link_targets "${_import_target}")
  endforeach() 
  
  cxxModules_hasValue(_engaged_libraries "${libraries}")
  if (_engaged_libraries)
    list(APPEND _link_targets ${libraries})
  endif()
  
  list(REMOVE_DUPLICATES _link_targets)
  
  set(${out_var} "${_link_targets}" PARENT_SCOPE)
endfunction()

# ============================================================
# cxxModules_resolveLibType(out_var lib_type module_name context)
# ------------------------------------------------------------
# Resolve and validate the library type for a module target
# by normalizing the input and enforcing membership in the
# supported set.
# ============================================================
function(cxxModules_resolveLibType out_var lib_type module_name context)
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

# ============================================================
# cxxModules_registerModule(module_name context)
# ------------------------------------------------------------
# Register a module in the global module registry enforcing
# uniqueness in the CXXMODULES_REGISTERED_MODULES list.
# ============================================================
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
