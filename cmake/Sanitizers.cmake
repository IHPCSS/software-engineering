# cmake/Sanitizers.cmake
# Instrument all compilation units with a runtime sanitiser.
#
# Activate via:
#   cmake -DSANITIZE=address    AddressSanitizer  — buffer overruns, use-after-free, leaks
#   cmake -DSANITIZE=memory     MemorySanitizer   — reads of uninitialised memory
#   cmake -DSANITIZE=thread     ThreadSanitizer   — data races between threads
#   cmake -DSANITIZE=undefined  UBSanitizer       — undefined behaviour (integer overflow, etc.)
#
# Constraints:
#   - ASan and TSan cannot be combined.
#   - MSan cannot be combined with ASan or TSan.
#   - MSan requires Clang; it will not work correctly with GCC.
#   - MSan requires that EVERY linked library (including the C++ standard
#     library) is itself compiled with -fsanitize=memory.  On a typical Linux
#     system the standard library is NOT instrumented, which causes false
#     positives when uninstrumented library code writes to memory that MSan
#     then sees as uninitialised.  The gold standard is to build against a
#     fresh libc++ compiled with -fsanitize=memory; see the LLVM docs for
#     instructions.  In CI we use it as a best-effort check of our own code.

set(SANITIZE "" CACHE STRING
    "Runtime sanitiser: address | memory | thread | undefined | (empty)")

if(SANITIZE)
    if(SANITIZE STREQUAL "memory" AND
       NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR
            "MemorySanitizer requires Clang.  "
            "Re-configure with -DCMAKE_CXX_COMPILER=clang++.")
    endif()

    # -fno-omit-frame-pointer gives readable stack traces at negligible cost.
    set(_san_flags "-fsanitize=${SANITIZE} -fno-omit-frame-pointer")

    # Apply to both compilation and linking.  We follow the same global-flag
    # style as CompilerFlags.cmake so every target in the project is covered.
    string(APPEND CMAKE_CXX_FLAGS           " ${_san_flags}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS    " ${_san_flags}")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " ${_san_flags}")

    message(STATUS "Sanitiser enabled: -fsanitize=${SANITIZE}")
    unset(_san_flags)
endif()
