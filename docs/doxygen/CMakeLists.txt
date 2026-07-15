find_package(Doxygen)

if(DOXYGEN_FOUND)
    # To avoid having to remember to update the documentation
    # version manually, we generate the Doxyfile from 
    # Doxyfile.cmakein, and set a few variables automatically
    # from CMake settings.

    set(DOXYGEN_INPUT_PATH "${CMAKE_SOURCE_DIR}/src/laplace")

    # If we found the "dot" program when we searched for doxygen,
    # enable it
    if(${DOXYGEN_DOT_FOUND})
        set(DOXYGEN_HAVE_DOT "YES")
    else()
        set(DOXYGEN_HAVE_DOT "NO")
    endif()

    configure_file(Doxyfile.cmakein Doxyfile)    

    add_custom_target(doxygen
                      COMMAND ${DOXYGEN_EXECUTABLE}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Building Doxygen source documentation"
                      VERBATIM)

endif(DOXYGEN_FOUND)
