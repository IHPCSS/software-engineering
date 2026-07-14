src/CMakeLists.txtif(BUILD_TESTS)
    # enable_testing() is called in the root CMakeLists.txt so that CTest can
    # discover tests regardless of which subdirectory they are defined in.
    add_subdirectory(external/googletest)
    include(TestMacros)
endif()

# This is the (for now empty) target to which all
# the actual test binaries will be added. By always adding
# this target (even if tests are not enabled), cmake will correctly
# tell you there are no tests available if you try to run them anyway.
add_custom_target(tests)

# This is a custom target that actually runs the tests
add_custom_target(check
                  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                  COMMENT "Running all tests"
                  USES_TERMINAL VERBATIM
                  DEPENDS tests)

configure_file(config.h.cmakein config.h)


add_subdirectory(software-engineering)


