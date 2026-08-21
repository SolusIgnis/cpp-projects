# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# CXXModules-Internal.cmake
# =================================================================

if(NOT _CXXMODULES_INCLUDED
   AND
   NOT _ADD_METAMODULE_INTERNAL_INCLUDED
   AND
   NOT _ADD_NAMED_MODULE_INTERNAL_INCLUDED)
  message(FATAL_ERROR "CXXModules-Internal.cmake is internal and must not be included directly.")
endif()

include_guard(GLOBAL)

# =================================================================
# Globals for where the CMake module lives
# =================================================================

set(CXXMODULES_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")
set(CXXMODULES_TEMPLATES_DIR "${CXXMODULES_ROOT_DIR}/templates" CACHE INTERNAL "")

# =================================================================
# Initialize the CACHE INTERNAL global registry list to empty.
# =================================================================

unset(CXXMODULES_REGISTERED_MODULES CACHE)

# =================================================================
# Private Helpers
# =================================================================

# ============================================================
# MACRO: cxxModules_resolveContext(out_var provided_context)
# ------------------------------------------------------------
# Resolve a human-readable context label for error reporting.
# Uses provided_context if set, otherwise falls back to the
# current CMake function name.
# ============================================================
macro(cxxModules_resolveContext out_var provided_context)
  if (provided_context)
    set(${out_var} "${provided_context}")
  else()
    set(${out_var} "${CMAKE_CURRENT_FUNCTION}")
  endif()
endmacro()

# =================================================================
# Semantic Presence Layer
# -----------------------------------------------------------------
# Imposes a `null` abstraction over CMake's untyped string
# values to distinguish between disengaged "semantically null"
# states and engaged values (including "false" and "0") that
# must be preserved for interpretation by higher layers.
# =================================================================

# ============================================================
# cxxModules_hasValue(out_var input)
# ------------------------------------------------------------
# Determine whether a value is present (i.e., not semantically
# null) in the CMake evaluation model.
#
# This function behaves like a null check or
# std::optional::has_value():
#   - TRUE  → a value is present (engaged)
#   - FALSE → no value is present (null / disengaged)
#
# This is a pre-interpretation check and does not determine
# what the value means--only whether a semantic value exists.
# ============================================================
function(cxxModules_hasValue out_var input)
  string(TOUPPER "${input}" input)
  if (input
      OR "${input}" STREQUAL "0"
      OR "${input}" STREQUAL "FALSE"
      OR "${input}" STREQUAL "OFF"
      OR "${input}" STREQUAL "NO")
    set(${out_var} "TRUE" PARENT_SCOPE)
  else()
    set(${out_var} "FALSE" PARENT_SCOPE)
  endif()
endfunction()

# ============================================================
# cxxModules_setWithDefault(out_var input default)
# ------------------------------------------------------------
# Set output to `input` if present; otherwise use default.
# ============================================================
function(cxxModules_setWithDefault out_var input default)
  cxxModules_hasValue(_engaged_input "${input}")
  if (_engaged_input)
    set(${out_var} "${input}" PARENT_SCOPE)
  else()
    set(${out_var} "${default}" PARENT_SCOPE)
  endif()
endfunction()

# =================================================================
# Validation Layer
# -----------------------------------------------------------------
# Enforces syntactic and semantic correctness of module names.
# =================================================================

# ============================================================
# cxxModules_validateModuleNameNoRepeatedPrefix(module_name context)
# ------------------------------------------------------------
# Validate that a module name does not begin with two
# identical components (e.g. "a.a", "a.a.b.c", or "base.base")
# in order to prevent name clashes with the stutter form of
# alias targets generated for single-component module names.
#
# NOTE: This restriction should have minimal practical impact
# for modules with meaningful names.
# ============================================================
function(cxxModules_validateModuleNameNoRepeatedPrefix module_name context)
  cxxModules_resolveContext(context "${context}")
  
  string(REPLACE "." ";" _parts "${module_name}")
  list(LENGTH _parts _len)

  if (_len GREATER 1)
    list(GET _parts 0 _first)
    list(GET _parts 1 _second)

    if ("${_first}" STREQUAL "${_second}")
      message(FATAL_ERROR "${context}(${module_name}): module names may not begin with two identical components")
    endif()
  endif()
endfunction()

# ============================================================
# cxxModules_validateModuleNameToken(module_name context)
# ------------------------------------------------------------
# Validate that a module name matches the allowed token
# grammar:
#           [A-Za-z_][A-Za-z0-9_]*(\.[A-Za-z_][A-Za-z0-9_]*)*
#           <identifier>[.<identifier>]*
# ============================================================
function(cxxModules_validateModuleNameToken module_name context)
  cxxModules_resolveContext(context "${context}")
  
  if (NOT module_name MATCHES "^[A-Za-z_][A-Za-z0-9_]*(\\.[A-Za-z_][A-Za-z0-9_]*)*$")
    message(FATAL_ERROR "${context}(${module_name}): invalid module name '${module_name}'")
  endif()

  cxxModules_validateModuleNameNoRepeatedPrefix("${module_name}" "${context}")
endfunction()

# ============================================================
# cxxModules_validateModuleName(module_name context)
# ------------------------------------------------------------
# Validate that a module name is non-empty, syntactically
# valid, and does not already exist as a CMake target.
# ============================================================
function(cxxModules_validateModuleName module_name context)
  cxxModules_resolveContext(context "${context}")
  
  if ("${module_name}" STREQUAL "")
    message(FATAL_ERROR "${context}: module name is required as first argument")
  endif()
  
  if (TARGET "${module_name}")
    message(FATAL_ERROR "${context}(${module_name}): target '${module_name}' already exists")
  endif()
  
  cxxModules_validateModuleNameToken("${module_name}" "${context}")
endfunction()

# =================================================================
# Structural Decomposition Layer
# -----------------------------------------------------------------
# Breaks module identifiers into hierarchical components and
# derived relationships.
# =================================================================

# ============================================================
# cxxModules_splitModuleName(out_list module_name context)
# ------------------------------------------------------------
# Split a dotted module name into its hierarchical components.
# Example: a.b.c → "a;b;c" interpreted as [a, b, c]
# ============================================================
function(cxxModules_splitModuleName out_list module_name context)
  cxxModules_resolveContext(context "${context}")
  
  cxxModules_validateModuleNameToken("${module_name}" "${context}")

  string(REPLACE "." ";" _parts "${module_name}")
  set(${out_list} "${_parts}" PARENT_SCOPE)
endfunction()

# ============================================================
# cxxModules_parentModule(out_var module_name context)
# ------------------------------------------------------------
# Compute the immediate parent module of a dotted module name
# or empty if the module has no parent.
# ============================================================
function(cxxModules_parentModule out_var module_name context)
  cxxModules_resolveContext(context "${context}")

  cxxModules_splitModuleName(_parts "${module_name}" "${context}")
  
  list(LENGTH _parts _len)

  if (_len LESS 2)
    set(${out_var} "" PARENT_SCOPE)
    return()
  endif()

  list(POP_BACK _parts)
  string(JOIN "." _parent ${_parts})
  set(${out_var} "${_parent}" PARENT_SCOPE)
endfunction()

# ============================================================
# cxxModules_coreName(out_var module_name context)
# ------------------------------------------------------------
# Extract the final segment of a dotted module name.
# ============================================================
function(cxxModules_coreName out_var module_name context)
  cxxModules_resolveContext(context "${context}")

  cxxModules_splitModuleName(_parts "${module_name}" "${context}")
  list(GET _parts -1 _leaf)
  set(${out_var} "${_leaf}" PARENT_SCOPE)
endfunction()

# =================================================================
# Representation Mapping Layer
# -----------------------------------------------------------------
# Converts between internal module names and external CMake alias
# forms used in C++ module syntax.
# =================================================================

# ============================================================
# cxxModules_moduleToAlias(out_var module_name context)
# ------------------------------------------------------------
# Convert a dotted module name into a CMake target alias.
# Example: a.b → a::b, a → a::a (stutter form)
# ============================================================
function(cxxModules_moduleToAlias out_var module_name context)
  cxxModules_resolveContext(context "${context}")
  
  cxxModules_validateModuleNameToken("${module_name}" "${context}")

  string(FIND "${module_name}" "." _dot_index)

  if (_dot_index EQUAL -1)
    # No dot → stutter
    set(_alias "${module_name}::${module_name}")
  else()
    string(REPLACE "." "::" _alias "${module_name}")
  endif()

  set(${out_var} "${_alias}" PARENT_SCOPE)
endfunction()

# ============================================================
# cxxModules_aliasToModule(out_var alias_name context)
# ------------------------------------------------------------
# Convert a CMake target alias back into a dotted module name.
# (Assumes alias is produced by cxxModules_moduleToAlias and
# follows a strict bijective mapping to module names.)
# Example: a::b → a.b, a::a → a (stutter form)
# ============================================================
function(cxxModules_aliasToModule out_var alias_name context)
  cxxModules_resolveContext(context "${context}")
  
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

    if ("${_first}" STREQUAL "${_second}")
      # Collapse stutter
      set(${out_var} "${_first}" PARENT_SCOPE)
      return()
    endif()
  endif()

  # General case: replace :: with .
  string(REPLACE "::" "." _module "${alias_name}")
  set(${out_var} "${_module}" PARENT_SCOPE)
endfunction()
