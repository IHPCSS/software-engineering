add_unit_test(GridTests   test-grid   grid.cpp)
target_link_libraries(test-grid   PRIVATE laplace_lib)

add_unit_test(BCTests     test-bc     boundary_conditions.cpp)
target_link_libraries(test-bc     PRIVATE laplace_lib)

add_unit_test(SolverTests test-solver solver.cpp)
target_link_libraries(test-solver PRIVATE laplace_lib)

# ── MemorySanitizer integration driver ────────────────────────────────────────
# A GoogleTest-free binary that exercises Grid, BoundaryConditions, and Solver
# directly.  See msan_driver.cpp for a full explanation of why this exists
# (short version: GoogleTest + MSan + uninstrumented libstdc++ produces
# persistent false positives that the suppression mechanism cannot reliably
# silence; sidestepping GoogleTest eliminates the problem at the source).
add_executable(msan-driver msan_driver.cpp)
target_link_libraries(msan-driver PRIVATE laplace_lib)
add_dependencies(tests msan-driver)
