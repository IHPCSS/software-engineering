/**
 * @file msan_driver.cpp
 * @brief GoogleTest-free integration driver used by the MemorySanitizer CI job.
 *
 * Why a separate driver?
 * ----------------------
 * MSan requires every linked library — including the C++ standard library —
 * to be compiled with -fsanitize=memory.  The libstdc++ shipped with
 * ubuntu-24.04 is NOT instrumented.  This causes a false positive during
 * GoogleTest's static initialisation:
 *
 *   BoolFromGTestEnv → FlagToEnvVar → testing::Message::operator<<
 *     → std::string ops in uninstrumented libstdc++
 *       → MSan: use-of-uninitialized-value
 *
 * The underlying mechanism is COMDAT folding: testing::Message::operator<< is
 * an inline template instantiated in both the GoogleTest static library
 * (compiled without MSan) and in our own test TUs (compiled with MSan).
 * The linker picks the instrumented copy from our TUs, so GoogleTest's init
 * code ends up calling instrumented code that reads uninstrumented libstdc++
 * memory.  The MSan suppression mechanism cannot reliably intercept this on
 * all toolchain/platform combinations.
 *
 * This driver exercises Grid, BoundaryConditions, and Solver directly with no
 * GoogleTest dependency, so the false positive never arises.  All real
 * uninitialised-memory reads in our library code are still caught.
 */

#include "laplace/boundary_conditions.h"
#include "laplace/grid.h"
#include "laplace/solver.h"

#include <cstdlib>
#include <iostream>

int main()
{
    // 12 × 12 grid (10 × 10 interior cells).
    // CornerHeatBC sets boundary values; interior converges in < 150 iterations.
    laplace::Grid         grid(12, 12, 0.0);
    laplace::CornerHeatBC bc;

    const auto [iterations, residual, converged] =
        laplace::solve(grid, bc, /*tolerance=*/1e-4, /*max_iter=*/2000);

    std::cout << (converged ? "converged" : "did NOT converge")
              << " after " << iterations << " iterations"
              << "  residual=" << residual << '\n';

    return converged ? EXIT_SUCCESS : EXIT_FAILURE;
}
