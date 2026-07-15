#
# This file was originally written for GROMACS, but
# just as the other CMake configuration files you can 
# redistributed it freely under the BSD 3-clause license.
#

include(CMakeParseArguments)

function (add_gtest_executable EXENAME)
    cmake_parse_arguments(ARG "${_options}" "" "" ${ARGN})
    set(_source_files ${ARG_UNPARSED_ARGUMENTS})
    include_directories(BEFORE SYSTEM ${CMAKE_SOURCE_DIR}/src/external/googletest/include)
    # EXCLUDE_FROM_ALL means we only build this binary if we really ask for the tests
    add_executable(${EXENAME} EXCLUDE_FROM_ALL ${_source_files})
    target_link_libraries(${EXENAME} PRIVATE ${GOOGLETEST_LIBRARIES})
    add_dependencies(${EXENAME} googletest)
endfunction()

function (register_googletest_test NAME EXENAME)
    set(_xml_path ${CMAKE_BINARY_DIR}/Testing/Temporary/${NAME}.xml)
    add_test(NAME ${NAME}
             COMMAND $<TARGET_FILE:${EXENAME}> --gtest_output=xml:${_xml_path})
    # Label all tests with "UnitTest" for time reporting, and set a generous timeout (300 seconds)
    set_tests_properties(${NAME} PROPERTIES LABELS "UnitTest")
    set_tests_properties(${NAME} PROPERTIES TIMEOUT 300)
    add_dependencies(tests ${EXENAME})
endfunction ()

function (add_unit_test NAME EXENAME)
    add_gtest_executable(${EXENAME} ${ARGN})
    register_googletest_test(${NAME} ${EXENAME})
endfunction()
