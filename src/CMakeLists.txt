if(BUILD_TESTS)
    # enable_testing() is called in the root CMakeLists.txt.
    add_subdirectory(external/googletest)
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
