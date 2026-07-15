#
# Coverage.cmake — gcov/lcov code-coverage support
#
# Adds the `coverage` build target, which:
#   1. Runs all CTest tests
#   2. Collects coverage data with lcov
#   3. Strips system headers and GoogleTest sources
#   4. Renders an HTML report under <build>/coverage-report/
#
# Usage:
#   cmake -DBUILD_TESTS=ON -DBUILD_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
#   cmake --build . --target tests    # compile test executables
#   cmake --build . --target coverage # run tests and generate report
#   open <build>/coverage-report/index.html
#
# Requirements:
#   - GCC or Clang (MSVC does not support --coverage)
#   - lcov and genhtml  (brew install lcov  /  apt install lcov)
#

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING
        "BUILD_COVERAGE is ON but CMAKE_BUILD_TYPE is '${CMAKE_BUILD_TYPE}'. "
        "Coverage data from optimised builds can be misleading. "
        "Consider: -DCMAKE_BUILD_TYPE=Debug")
endif()

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    message(FATAL_ERROR
        "Code coverage requires GCC or Clang. "
        "Current compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Instrument every translation unit compiled in this build.
add_compile_options(--coverage)
add_link_options(--coverage)

# Locate lcov / genhtml — REQUIRED so CMake errors early if they are missing.
find_program(LCOV_EXECUTABLE    lcov    REQUIRED
    DOC "lcov coverage data collector (part of the Linux Test Project lcov package)")
find_program(GENHTML_EXECUTABLE genhtml REQUIRED
    DOC "genhtml HTML report generator (part of the Linux Test Project lcov package)")

set(_coverage_info "${CMAKE_BINARY_DIR}/coverage.info")
set(_coverage_dir  "${CMAKE_BINARY_DIR}/coverage-report")

add_custom_target(coverage
    # Step 1: run all tests via CTest.
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure

    # Step 2: collect .gcda data produced by the test run.
    COMMAND ${LCOV_EXECUTABLE}
            --capture
            --directory ${CMAKE_BINARY_DIR}
            --output-file ${_coverage_info}

    # Step 3: strip noise — system headers and GoogleTest sources.
    COMMAND ${LCOV_EXECUTABLE}
            --remove ${_coverage_info}
            "/usr/*"
            "*/external/googletest/*"
            --output-file ${_coverage_info}
            --ignore-errors unused

    # Step 4: render the HTML report.
    COMMAND ${GENHTML_EXECUTABLE}
            ${_coverage_info}
            --output-directory ${_coverage_dir}

    COMMENT "Coverage report → ${_coverage_dir}/index.html"

    # Ensure all test executables are built before we try to run them.
    DEPENDS tests

    USES_TERMINAL
    VERBATIM
)
