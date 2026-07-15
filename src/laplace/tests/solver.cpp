/**
 * @file test_solver.cpp
 * @brief Unit tests for the Jacobi solver.
 *
 * Coverage targets
 * ================
 * solver.cpp
 *   - Initial bc.apply(grid) and bc.apply(next) calls
 *   - While-loop: entry, convergence exit (residual ≤ tolerance), and
 *     max-iter exit (iterations ≥ max_iter with residual still > tolerance)
 *   - Jacobi sweep: inner loops, row-pointer hoisting
 *   - if (diff > residual) residual = diff
 *       TRUE  branch — first cell where diff exceeds the running maximum
 *       FALSE branch — subsequent cells whose diff ≤ current maximum
 *   - std::swap(grid, next) and SolverResult return
 *
 * Branch coverage notes
 * =====================
 * UniformBCConvergesInOneIteration
 *   All interior cells have diff = 0.0, so  if (0.0 > 0.0)  → FALSE branch.
 *
 * MaxIterExceeded
 *   The bottom interior rows of the 5×5 grid change from 0 to 25 °C.
 *   First cell in the hot row: diff=25 > residual=0 → TRUE branch.
 *   Subsequent cells in the same row: diff=25 = residual=25 → FALSE branch.
 */

#include "laplace/boundary_conditions.h"
#include "laplace/grid.h"
#include "laplace/solver.h"

#include <gtest/gtest.h>

namespace laplace {
namespace test {

// ── Convergence: trivial uniform case ─────────────────────────────────────────
//
// All four edges at the same temperature T means the exact solution is
// u = T everywhere.  If the initial grid also holds T, every cell update
// produces out[i] = 0.25*(T+T+T+T) = T, so diff = 0 and the solver exits
// after exactly one sweep.

TEST(SolverTest, UniformBCConvergesInOneIteration)
{
    constexpr double T = 1.0;
    Grid g(5, 5, T);
    ConstantBC bc(T, T, T, T);

    auto result = solve(g, bc, /*tolerance*/0.01, /*max_iter*/100);

    EXPECT_TRUE(result.converged);
    EXPECT_EQ(result.iterations, 1u);
    EXPECT_DOUBLE_EQ(result.residual, 0.0);

    // Every cell must still hold T after the solve.
    for (std::size_t j = 0; j < g.ny(); ++j)
        for (std::size_t i = 0; i < g.nx(); ++i)
            EXPECT_DOUBLE_EQ(g(i, j), T) << "  at (" << i << "," << j << ")";
}

// ── Convergence: exact single-interior-cell solution ─────────────────────────
//
// A 3×3 grid has exactly one interior cell: (1, 1).
// With top=4, rest=0, the Laplace solution is the average of the four
// boundary neighbours: u(1,1) = 0.25*(0+0+0+4) = 1.0.
//
// Iteration 1: interior cell is updated from 0 → 1.0  (diff = 1.0)
// Iteration 2: neighbours unchanged, update again → 1.0 (diff = 0.0) → converged

TEST(SolverTest, SingleInteriorCellExactSolution)
{
    Grid g(3, 3, 0.0);
    ConstantBC bc(/*top*/4.0, /*bottom*/0.0, /*left*/0.0, /*right*/0.0);

    auto result = solve(g, bc);

    EXPECT_TRUE(result.converged);
    EXPECT_DOUBLE_EQ(g(1, 1), 1.0);
}

// ── Convergence: non-trivial boundary conditions ──────────────────────────────
//
// Uses the original PSC/IHPCSS demo boundary conditions.  We don't know the
// exact solution, but we can assert that the solver converges and that the
// perimeter cells retain their boundary values afterwards.

TEST(SolverTest, CornerHeatBCConverges)
{
    Grid g(20, 20, 0.0);
    CornerHeatBC bc;

    auto result = solve(g, bc, /*tolerance*/0.01, /*max_iter*/5000);

    EXPECT_TRUE(result.converged);
    EXPECT_LE(result.residual, 0.01);

    // Bottom-right corner must be 100 (owned by bottom ramp).
    EXPECT_DOUBLE_EQ(g(19, 0), 100.0);
    // Top-left corner must be 0.
    EXPECT_DOUBLE_EQ(g(0, 19), 0.0);
}

// ── Max-iter exceeded ─────────────────────────────────────────────────────────
//
// One sweep is nowhere near enough to converge from an all-zero initial guess
// with a hot top boundary.  The solver must return converged=false and report
// the correct iteration count.
//
// Side effect on branch coverage: the hot row near the top will produce cells
// with diff=25 > residual=0 (TRUE branch), and then subsequent cells in that
// row with diff=25 = residual=25 (FALSE branch), covering both arms of the
// if (diff > residual) conditional.

TEST(SolverTest, MaxIterExceeded)
{
    Grid g(5, 5, 0.0);
    // Top=100, rest=0.  After one sweep the row below the top boundary goes
    // from 0 → 25 °C — far from converged.
    ConstantBC bc(/*top*/100.0, /*bottom*/0.0, /*left*/0.0, /*right*/0.0);

    auto result = solve(g, bc, /*tolerance*/1e-10, /*max_iter*/1);

    EXPECT_FALSE(result.converged);
    EXPECT_EQ(result.iterations, 1u);
    EXPECT_GT(result.residual, 1e-10);
}

// ── SolverResult fields ───────────────────────────────────────────────────────
//
// Verify that the three struct fields carry the expected semantics:
//   converged  ↔  residual ≤ tolerance
//   residual   is non-negative
//   iterations is in [1, max_iter]

TEST(SolverTest, SolverResultFieldsAreConsistent)
{
    Grid g(5, 5, 0.0);
    ConstantBC bc(50.0, 0.0, 0.0, 0.0);

    auto result = solve(g, bc, /*tolerance*/0.01, /*max_iter*/4000);

    EXPECT_GE(result.iterations, 1u);
    EXPECT_LE(result.iterations, 4000u);
    EXPECT_GE(result.residual,  0.0);
    EXPECT_EQ(result.converged, result.residual <= 0.01);
}

} // namespace test
} // namespace laplace
