# Sanitized Debug build presets use -O1.
# Users requiring a different optimization level should
# configure the build manually rather than using the
# provided presets.


# ===========================================================================
# Configure LLVM sanitizers.
# ===========================================================================
set(
  LLVM_SANITIZER_KIND
  ""
  CACHE STRING
  "Value passed to Clang's -fsanitize="
)

if(LLVM_SANITIZER_KIND)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR
      "LLVM_SANITIZER_KIND requires a Clang compiler."
    )
  endif()

  message(STATUS
    "Enabling LLVM sanitizers: ${LLVM_SANITIZER_KIND}"
  )

  string(APPEND CMAKE_CXX_FLAGS_DEBUG " -O1")

  add_compile_options(
    -fno-omit-frame-pointer
    -fsanitize=${LLVM_SANITIZER_KIND}
  )

  add_link_options(
    -fsanitize=${LLVM_SANITIZER_KIND}
  )

endif()

# ===========================================================================
# Configure GCC sanitizers.
# ===========================================================================

# Currently unsupported.

# ===========================================================================
# Configure MSVC sanitizers.
# ===========================================================================

# Currently unsupported.
