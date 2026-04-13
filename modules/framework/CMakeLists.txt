# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
# ===========================================================================
# //modules/framework/CMakeLists.txt
# ===========================================================================

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

trace_me()

# ===========================================================================
# Add submodule directories in dependency order.
# ===========================================================================

# Add the `framework.NULL` metamodule
# add_subdirectory("NULL")

# ===========================================================================
# Add generated metamodule from submodules registered above.
# ===========================================================================

# add_metamodule(framework)
