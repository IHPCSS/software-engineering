/**
 * @file main.cpp
 * @brief Command-line driver for the 2-D Laplace solver.
 *
 * Reproduces the classic PSC/IHPCSS benchmark problem: a plate where the
 * left and top edges are held at 0 °C and the right and bottom edges
 * increase linearly to 100 °C at the bottom-right corner.
 */

#include "laplace/boundary_conditions.h"
#include "laplace/grid.h"
#include "laplace/solver.h"

#include <chrono>
#include <iostream>

int main()
{
    // ── Problem parameters ─────────────────────────────────────────────────
    // Grid size chosen to be easily divisible by typical HPC node core counts.
    // The +2 accounts for the boundary ring; interior cells are ROWS × COLUMNS.
    constexpr std::size_t COLUMNS   = 672;
    constexpr std::size_t ROWS      = 672;
    constexpr double      TOLERANCE = 0.01;
    constexpr std::size_t MAX_ITER  = 4000;

    std::cout << "Grid: " << ROWS << " x " << COLUMNS << "\n"
              << "Tolerance: " << TOLERANCE
              << "  Max iterations: " << MAX_ITER << "\n";

    // ── Setup ──────────────────────────────────────────────────────────────
    laplace::Grid         grid(COLUMNS + 2, ROWS + 2, 0.0);
    laplace::CornerHeatBC bc;

    // ── Solve ──────────────────────────────────────────────────────────────
    const auto t0     = std::chrono::steady_clock::now();
    const auto result = laplace::solve(grid, bc, TOLERANCE, MAX_ITER);
    const double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    // ── Report ─────────────────────────────────────────────────────────────
    std::cout << (result.converged ? "Converged" : "Did NOT converge")
              << " after " << result.iterations << " iterations"
              << "  (max change = " << result.residual << ")\n"
              << "Total time: " << elapsed << " seconds\n";

    return result.converged ? 0 : 1;
}
