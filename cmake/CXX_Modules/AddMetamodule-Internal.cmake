# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# AddMetamodule-Internal.cmake
# ============================================================

if(NOT _ADD_METAMODULE_INCLUDED)
  message(FATAL_ERROR "AddNamedModule-Internal.cmake is internal and must not be included directly.")
endif()

include_guard(GLOBAL)

# ============================================================
# Let our includes see AddNamedModule-Internal.cmake was included first.
# ============================================================

set(_ADD_METAMODULE_INTERNAL_INCLUDED TRUE)

# ============================================================
# Include the internal implementation helpers
# ============================================================

include(${CMAKE_CURRENT_LIST_DIR}/CXXModules-Internal.cmake)

# ============================================================
# Private Helpers
# ============================================================

function(cxxModules_appendFlagIfSet list_var key value)
  if (value)
    list(APPEND ${list_var} ${key})
    set(${list_var} "${${list_var}}" PARENT_SCOPE)
  endif()
endfunction()

function(cxxModules_appendIfSet list_var key value)
  if (value)
    list(APPEND ${list_var} ${key} ${value})
    set(${list_var} "${${list_var}}" PARENT_SCOPE)
  endif()
endfunction()
