src/software-engineering/program/CMakeLists.txt# Build the binary 'laplace' from the source file (now renamed to be a c++ source)
add_executable(laplace laplace_serial.cpp)

# Link against MPI/OpenMP/OpenACC imported targets when they are enabled.
# Using imported targets (MPI::MPI_CXX, OpenMP::OpenMP_CXX) is the modern
# CMake approach: they carry include directories, compile flags, and link
# libraries automatically, so we do not need to manipulate global variables.
if(MPI AND MPI_CXX_FOUND)
    target_link_libraries(laplace PRIVATE MPI::MPI_CXX)
endif()
if(OPENMP AND OpenMP_CXX_FOUND)
    target_link_libraries(laplace PRIVATE OpenMP::OpenMP_CXX)
endif()
if(OPENACC AND OpenACC_CXX_FOUND)
    target_link_libraries(laplace PRIVATE OpenACC::OpenACC_CXX)
endif()

install(TARGETS laplace RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
