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
# Include the internal implementation helpers
# ============================================================

include(${CMAKE_CURRENT_LIST_DIR}/AddNamedModule-Internal.cmake)

# ============================================================
# Public API
# ============================================================

# ============================================================
# add_named_module(name [ARGN])
# ------------------------------------------------------------
# This is the core of the module system, bridging logical C++
# module names to concrete CMake targets. It defines a module
# as a validated library target and manages its interface
# unit, partitions, and dependencies while automatically
# handling alias generation, global registration, and the
# configuration of compilation, linkage, and optional tooling
# and testing behaviors.
#
# Arguments:
#   name                   Canonical module name as declared by
#                          e.g., `export module foo.bar.baz;`.
#                          Maps to target `foo.bar.baz` and
#                          alias `foo::bar::baz`.
#
# Required:
#   LIB_TYPE <type>        STATIC | OBJECT | SHARED (warns)
#
# Optional:
#   INTERFACE_UNIT <f>     Primary module interface unit
#                          source file. Defaults to
#                          'src/<name>.cppm'. Must refer, if
#                          specified, to exactly one file
#                          present in the filesystem.
#   PARTITIONS <f...>      Module partition interface unit
#                          source files registered on the
#                          target via FILE_SET CXX_MODULES.
#   IMPLEMENTATIONS <f...> Module implementation units (.cpp)
#                          added as PRIVATE sources.
#   IMPORTS <mods...>      Dotted module names of import
#                          dependencies. Resolves to library
#                          target aliases for PUBLIC linking
#                          (e.g., foo.bar -> foo::bar).
#   LINK_LIBRARIES <l...>  Raw library targets to link against.
#   BASE_DIRS <d...>       Base directories for module file
#                          sets. Defaults to 'src'.
#   STD <num>              C++ standard (min 23, default 23).
#   NO_TESTS               Suppress automatic test discovery
#                          via 'add_tests_for_module'.
#   NO_TOOLING             Suppress automatic tooling usage
#                          via 'register_tooling_target'.
#
# Notes:
#   - Registers the module in the global registry for
#     metamodule support.
#   - The alias target is the canonical linking name for
#     consumers.
#   - Aborts by FATAL ERROR on any validation failure or
#     duplicate registration attempt.
#   - IMPORTS must refer to modules defined before this call.
#
# Example:
#   add_named_module(foo.bar
#     LIB_TYPE STATIC
#     INTERFACE_UNIT src/foo.bar.cppm
#     PARTITIONS
#       src/foo.bar-part1.cppm
#       src/foo.bar-part2.cppm
#     IMPLEMENTATIONS
#       src/impl/foo.bar-part1-impl.cpp
#       src/impl/foo.bar-part2-impl.cpp
#     IMPORTS
#       foo.baz
#       quux
#     LINK_LIBRARIES
#       some::external_lib
#   )
# ============================================================
function(add_named_module name)

  cmake_parse_arguments(NM_ARG
    "NO_TESTS;NO_TOOLING"
    "INTERFACE_UNIT;LIB_TYPE;STD;_CONTEXT"
    "BASE_DIRS;PARTITIONS;IMPLEMENTATIONS;IMPORTS;LINK_LIBRARIES;TEST_DEPENDENCIES"
    ${ARGN}
  )

  cxxModules_resolveContext(context "${NM_ARG__CONTEXT}")
  
  if (NOT DEFINED NM_ARG_LIB_TYPE)
    message(FATAL_ERROR "${context}(${name}): LIB_TYPE is required.")
  endif()
  
  # --- Validation ---
  
  cxxModules_validateModuleName("${name}" "${context}")
  cxxModules_resolveInterfaceUnit(_interface_unit "${NM_ARG_INTERFACE_UNIT}" "${name}" "${context}")
  cxxModules_resolveLibType(_lib_type "${NM_ARG_LIB_TYPE}" "${name}" "${context}")
  
  # --- Default BASE_DIRS is ./src/ ---
  cxxModules_setWithDefault(_base_dirs "${NM_ARG_BASE_DIRS}" "src")

  # --- Target + alias ---
  add_library("${name}" "${_lib_type}")
    
  cxxModules_moduleToAlias(_alias "${name}" "${context}")

  if (TARGET "${_alias}")
    message(FATAL_ERROR "${context}(${name}): target '${_alias}' already exists")
  endif()
  
  add_library("${_alias}" ALIAS "${name}")

  # --- Tooling ---
  if (NOT NM_ARG_NO_TOOLING)
    if (COMMAND register_tooling_target)
      register_tooling_target("${name}")
    else()
      message(WARNING "Function 'register_tooling_target' not found. Did you forget to 'include(ToolingInfrastructure)'?")
    endif()
  endif()

  # --- Language level ---
  cxxModules_setWithDefault(_cxx_std "${NM_ARG_STD}" 23)
  cxxModules_validateStd("${_cxx_std}" "${context}")

  target_compile_features("${name}"
    PUBLIC "cxx_std_${_cxx_std}"
  )

  # --- Module units ---
  set(_module_files "${_interface_unit}")
  list(APPEND _module_files ${NM_ARG_PARTITIONS})

  target_sources("${name}"
    PUBLIC
      FILE_SET CXX_MODULES
      BASE_DIRS "${_base_dirs}"
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

  cxxModules_collectLinkTargets(_link_targets "${NM_ARG_IMPORTS}" "${NM_ARG_LINK_LIBRARIES}" "${context}")
  
  if (_link_targets)
    target_link_libraries("${name}"
      PUBLIC
        ${_link_targets}
    )
  endif()

  # --- Tests ---
  if (NOT NM_ARG_NO_TESTS)
    if (COMMAND add_tests_for_module)
      if (NM_ARG_TEST_DEPENDENCIES)
        add_tests_for_module("${name}"
          DEPENDENCIES ${NM_ARG_TEST_DEPENDENCIES}
        )
      else()
        add_tests_for_module("${name}")
      endif()
    else()
      message(WARNING "${context}(${name}): Function 'add_tests_for_module' not found. Tests for module '${name}' may be unavailable. Did you forget to 'include(DiscoverTests)'?")
    endif()
  endif()
  
  cxxModules_registerModule("${name}" "${context}")

endfunction()
