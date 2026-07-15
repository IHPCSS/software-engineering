if(BUILD_TESTS)
    # enable_testing() is called in the root CMakeLists.txt.
    add_subdirectory(external/googletest)

    # GoogleTest is vendored external code compiled with our flags.  Two
    # adjustments are needed:
    #
    # 1. -Wno-error: our flags include -Werror, but pre-existing warnings in
    #    third-party source must not break the build.
    #
    # 2. -fno-sanitize=memory (MSan only): MSan requires every linked library
    #    to be instrumented.  libstdc++ on the CI runner is NOT instrumented,
    #    so any std::string use inside GoogleTest's static initialisation
    #    triggers false positives.  Compiling GoogleTest without MSan
    #    instrumentation eliminates those false positives while keeping our
    #    own library code fully checked.  The MSan runtime is still linked via
    #    the executable's linker flags, so real errors in our code are caught.
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
