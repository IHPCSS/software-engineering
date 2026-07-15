add_unit_test(GridTests   test-grid   grid.cpp)
target_link_libraries(test-grid   PRIVATE laplace_lib)

add_unit_test(BCTests     test-bc     boundary_conditions.cpp)
target_link_libraries(test-bc     PRIVATE laplace_lib)

add_unit_test(SolverTests test-solver solver.cpp)
target_link_libraries(test-solver PRIVATE laplace_lib)
