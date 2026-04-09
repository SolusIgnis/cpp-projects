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

function(cxxModules_collectRegisteredSubmodules out_var module_name context)
  set(_child_modules)
  
  foreach(_module IN LISTS CXXMODULES_REGISTERED_MODULES)
    cxxModules_parentModule(_parent "${_module}" "${context}")
    if("${_parent}" STREQUAL "${module_name}")
      list(APPEND _child_modules "${_module}")
    endif()
  endforeach()
  
  set(${out_var} "${_child_modules}" PARENT_SCOPE)
endfunction()

function(cxxModules_validateSubmodules submodules module_name context)
  # Metamodule must have at least one submodule
  if (NOT submodules)
    message(FATAL_ERROR "${context}(${module_name}): no submodules found for '${module_name}'. Ensure submodules are registered before adding parent metamodule or specify 'SUBMODULES' argument explicitly.")
  endif()

  # Metamodule must not list itself as a submodule
  list(FIND submodules "${module_name}" _self_index)
  if (NOT _self_index EQUAL -1)
    message(FATAL_ERROR "${context}(${module_name}): ${module_name} cannot include itself in SUBMODULES")
  endif()

  foreach(_submodule IN LISTS submodules)
    # Metamodule must be parent of submodules
    cxxModules_parentModule(_parent "${_submodule}" "${context}")
    if (NOT "${_parent}" STREQUAL "${module_name}")
      message(FATAL_ERROR "${context}(${module_name}): metamodule '${module_name}' is not the parent of submodule '${_submodule}'")
    endif()
    # Submodules must be valid targets
    if (NOT TARGET "${_submodule}")
      message(FATAL_ERROR "${context}(${module_name}): submodule '${_submodule}' is not a known target")
    endif()
  endforeach()
endfunction()

function(cxxModules_processSubmodules out_var submodules_arg module_name context)
  if (submodules_arg)
    set(_submodules "${submodules_arg}")
  else()
    cxxModules_collectRegisteredSubmodules(_submodules "${module_name}" "${context}")
  endif()

  list(REMOVE_DUPLICATES _submodules)
  # SUBMODULES are treated as an unordered set; ordering is normalized
  list(SORT _submodules)

  # --- Enforce metamodule invariants ---
  cxxModules_validateSubmodules("${_submodules}" "${module_name}" "${context}")
  set(${out_var} "${_submodules}" PARENT_SCOPE)
endfunction()

function(cxxModules_generateMetamoduleSourceCode out_interface_unit out_base_dirs module_name submodules)
  set(_gen_dir "${CMAKE_CURRENT_BINARY_DIR}/generated/metamodules")
  set(_gen_path "${_gen_dir}/${module_name}.cppm")

  file(MAKE_DIRECTORY "${_gen_dir}")

  # Prepare export lines and header comments
  set(_submodules_header_comment_lines "")
  set(_submodules_export_imports_lines "")
  foreach(_submodule IN LISTS submodules)
    list(APPEND _submodules_header_comment_lines " *   - `${_submodule}`")
    list(APPEND _submodules_export_imports_lines "export import ${_submodule}; ///< @see \"${_submodule}.cppm\"")
  endforeach()
  string(JOIN "\n" submodules_header_comment ${_submodules_header_comment_lines})
  string(JOIN "\n" submodules_export_imports ${_submodules_export_imports_lines})

  configure_file(
    "${CXXMODULES_TEMPLATES_DIR}/metamodule.cppm.in"
    "${_gen_path}"
    @ONLY
  )
    
  execute_process(COMMAND "${CMAKE_COMMAND}" -E cat "${_gen_path}")
  
  set(${out_interface_unit} "${_gen_path}" PARENT_SCOPE)
  set(${out_base_dirs} "${_gen_dir}" PARENT_SCOPE)
endfunction()

function(cxxModules_resolveMetamoduleInterface
  out_interface_unit
  out_base_dirs
  module_name
  submodules
  interface_unit_arg
  context
)
  cxxModules_resolveContext(context "${context}")

  if (interface_unit_arg AND NOT "${interface_unit_arg}" STREQUAL "GENERATED")
    set(_iface "${interface_unit_arg}")

    # Derive base dir from path
    get_filename_component(_base_dir "${_iface}" DIRECTORY)

  else()
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src" AND NOT "${interface_unit_arg}" STREQUAL "GENERATED")
      message(FATAL_ERROR
        "${context}(${module_name}): Found src/ subdirectory, but INTERFACE_UNIT was not specified.\n"
        "Either pass INTERFACE_UNIT explicitly to use it, remove the directory, or pass INTERFACE_UNIT GENERATED to ignore it."
      )
    endif()

    # --- Generate ---
    cxxModules_generateMetamoduleSourceCode(_iface _base_dir "${module_name}" "${submodules}")
  endif()
  
  set(${out_interface_unit} "${_iface}" PARENT_SCOPE)
  set(${out_base_dirs} "${_base_dir}" PARENT_SCOPE)
endfunction()
