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

function(add_named_module name)

  cmake_parse_arguments(NM_ARG
    "NO_TESTS"
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
  cxxModules_validateInterfaceUnit(_interface_unit "${NM_ARG_INTERFACE_UNIT}" "${name}" "${context}")
  cxxModules_validateLibType(_lib_type "${NM_ARG_LIB_TYPE}" "${name}" "${context}")
  
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
  if (COMMAND register_tooling_target)
    register_tooling_target("${name}")
  else()
    message(WARNING "Function 'register_tooling_target' not found. Did you forget to 'include(ToolingInfrastructure)'?")
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

endfunction()
