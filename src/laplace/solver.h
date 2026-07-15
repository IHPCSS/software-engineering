#pragma once

/**
 * @file solver.h
 * @brief Jacobi iterative solver for the 2-D Laplace equation.
 */

#include "laplace/boundary_conditions.h"
#include "laplace/grid.h"
#include <cstddef>

namespace laplace {

/**
 * @brief Summary of a completed solve.
 */
struct SolverResult {
    std::size_t iterations; ///< Number of Jacobi sweeps performed.
    double      residual;   ///< Maximum absolute change in the final sweep.
    bool        converged;  ///< True if residual fell below tolerance.
};

/**
 * @brief Solve ∇²u = 0 on @p grid using Jacobi iteration.
 *
 * ## Algorithm
 *
 * The Jacobi update for an interior cell `(i, j)` is:
 * @f[
 *   u^{(k+1)}_{i,j} = \tfrac{1}{4}\!\left(
 *     u^{(k)}_{i-1,j} + u^{(k)}_{i+1,j} +
 *     u^{(k)}_{i,j-1} + u^{(k)}_{i,j+1}
 *   \right)
 * @f]
 *
 * A double buffer is used: the sweep reads from @p grid and writes into a
 * private buffer; after each sweep the buffers are swapped (zero-copy).
 * The residual (maximum absolute cell change) is accumulated **inline**
 * during the sweep — not in a separate pass — so each iteration touches
 * the data exactly once.  Row pointers are hoisted outside the inner loop
 * to expose a stride-1 access pattern that the compiler can auto-vectorise.
 *
 * ## Extension points for students
 *
 * - **MPI**: partition rows across ranks; add a halo-exchange
 *   (`MPI_Sendrecv`) on the ghost rows before each sweep.
 * - **CUDA**: launch a 2-D thread block per interior cell; use
 *   double-buffering with device-side Grid instances.
 * - **Gauss-Seidel / SOR**: update @p grid in-place (drop the @p next
 *   buffer); introduce a relaxation factor ω > 1 for faster convergence.
 *
 * @param grid      In/out: initial guess on entry, solution on exit.
 *                  Must have nx ≥ 3 and ny ≥ 3 (at least one interior cell).
 * @param bc        Boundary conditions applied after every sweep.
 * @param tolerance Convergence threshold on the max cell change. Default: 0.01.
 * @param max_iter  Hard upper limit on the number of sweeps.   Default: 4000.
 * @return A SolverResult describing the outcome.
 */
SolverResult solve(Grid&                     grid,
                   const BoundaryConditions& bc,
                   double                    tolerance = 0.01,
                   std::size_t               max_iter  = 4000);

} // namespace laplace
