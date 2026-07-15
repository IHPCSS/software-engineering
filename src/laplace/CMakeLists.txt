add_library(laplace_lib STATIC
    grid.cpp
    boundary_conditions.cpp
    solver.cpp
)

# Expose src/ on the include path so consumers include as "laplace/grid.h".
target_include_directories(laplace_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

if(MPI AND MPI_CXX_FOUND)
    target_link_libraries(laplace_lib PUBLIC MPI::MPI_CXX)
endif()
if(OPENMP AND OpenMP_CXX_FOUND)
    target_link_libraries(laplace_lib PUBLIC OpenMP::OpenMP_CXX)
endif()
if(OPENACC AND OpenACC_CXX_FOUND)
    target_link_libraries(laplace_lib PUBLIC OpenACC::OpenACC_CXX)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
