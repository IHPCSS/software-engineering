if(BUILD_TESTS)
    # enable_testing() is called in the root CMakeLists.txt.
    add_subdirectory(external/googletest)

    # GoogleTest is vendored external code compiled with our flags.  Two
    # adjustments are needed:
    #
    # 1. -Wno-error: our flags include -Werror, but pre-existing warnings in
    #    third-party source must not break the build.
    #
    # 2. -fno-sanitize=memory (MSan only): attempt to exclude the googletest
    #    static library from MSan instrumentation.  This is a best-effort hint;
    #    it is insufficient on its own because inline template functions from
    #    gtest-message.h (e.g. testing::Message::operator<<) are also
    #    instantiated in our test TUs, which ARE instrumented, and the linker
    #    may pick those instrumented COMDAT copies when googletest's static init
    #    calls them.  The remaining false positives are suppressed at runtime
    #    via msan_suppressions.txt (see the msan CI job for details).
    foreach(_gtest_target googletest googletest_main)
        if(TARGET ${_gtest_target})
            target_compile_options(${_gtest_target} PRIVATE -Wno-error)
            if(SANITIZE STREQUAL "memory")
                target_compile_options(${_gtest_target} PRIVATE -fno-sanitize=memory)
            endif()
        endif()
    endforeach()
    unset(_gtest_target)

    include(TestMacros)

    # 'tests' is the custom target that all test executables depend on.
    # Build it explicitly with: cmake --build build --target tests
    add_custom_target(tests)

    # 'check' builds and immediately runs all tests.
    add_custom_target(check
                      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                      COMMENT "Running all tests"
                      USES_TERMINAL VERBATIM
                      DEPENDS tests)
endif()

configure_file(config.h.cmakein config.h)

add_subdirectory(laplace)
add_subdirectory(program)
