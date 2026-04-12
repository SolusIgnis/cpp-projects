# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
# ===========================================================================
# //modules/CMakeLists.txt
# ===========================================================================

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

trace_me()

# ===========================================================================
# Add top-level metamodule subdirectories in dependency layer order.
# ===========================================================================

# `base` modules have no non-`std` dependencies.
add_subdirectory("base")

# `tools` can depend on `base` but all other layers' tests etc. may depend on `tools`.
add_subdirectory("tools")

# `framework` depends on `base` but can be a dependency of every other layer.
add_subdirectory("framework")

# Add the `net` metamodule.
add_subdirectory("net")
