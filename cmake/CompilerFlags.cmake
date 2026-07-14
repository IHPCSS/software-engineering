include(CheckCXXCompilerFlag)

# Test C++ flags FLAGS, and set VARIABLE to true if they work. Also add the
# flags to CXXFLAGSVAR if they worked.
#
# Note: appending to CMAKE_CXX_FLAGS is a "global" approach that predates
# modern target-based CMake. It works well for a teaching project where all
# targets should share the same flags, but in a larger project you would
# instead create an INTERFACE library and use target_link_libraries().
macro(test_and_append_cxx_flag VARIABLE FLAGS CXXFLAGSVAR)
    if(NOT DEFINED ${VARIABLE})
        check_cxx_compiler_flag("${FLAGS}" ${VARIABLE})
    endif()
    if(${VARIABLE})
        set(${CXXFLAGSVAR} "${${CXXFLAGSVAR}} ${FLAGS}")
    endif()
endmacro()

# Enable a broad set of warnings. Good code should compile without warnings,
# and warnings often reveal real bugs.
test_and_append_cxx_flag(CXX_WALL     "-Wall"     CMAKE_CXX_FLAGS)
test_and_append_cxx_flag(CXX_WEXTRA  "-Wextra"   CMAKE_CXX_FLAGS)
test_and_append_cxx_flag(CXX_WPEDANTIC "-Wpedantic" CMAKE_CXX_FLAGS)

# Enable fast-math with gcc/clang (can be dangerous for some algorithms —
# it allows the compiler to reorder floating-point operations).
test_and_append_cxx_flag(CXX_FAST_MATH "-ffast-math" CMAKE_CXX_FLAGS)

# Try the -fast flag (used with Intel compilers).
# Clang only warns about this option rather than rejecting it, so we guard
# it with a compiler-ID check — a useful example of compiler-specific logic.
if(CMAKE_CXX_COMPILER_ID MATCHES "Intel|IntelLLVM")
    test_and_append_cxx_flag(CXX_FAST "-fast" CMAKE_CXX_FLAGS)
endif()
