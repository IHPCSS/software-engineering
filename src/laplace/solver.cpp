#include "laplace/solver.h"

#include <algorithm>
#include <cmath>

namespace laplace {

SolverResult solve(Grid&                     grid,
                   const BoundaryConditions& bc,
                   double                    tolerance,
                   std::size_t               max_iter)
{
    const std::size_t nx = grid.nx();
    const std::size_t ny = grid.ny();

    // Impose boundary conditions on the initial state.
    bc.apply(grid);

    // Allocate the second buffer for Jacobi double-buffering.
    // The sweep reads from 'grid' and writes to 'next'; the buffers are
    // swapped after each sweep via std::swap (zero-copy: only pointers move).
    Grid next(nx, ny, 0.0);
    bc.apply(next);

    double      residual   = tolerance + 1.0; // ensure at least one sweep
    std::size_t iterations = 0;

    while (iterations < max_iter && residual > tolerance) {

        residual = 0.0;

        // ── Jacobi sweep ───────────────────────────────────────────────────
        // Row pointers are hoisted outside the inner loop: the compiler sees
        // a plain stride-1 load/store pattern and can auto-vectorise the
        // inner loop over i without any index arithmetic in the hot path.
        for (std::size_t j = 1; j < ny - 1; ++j) {
            const double* above = grid.row(j + 1);
            const double* below = grid.row(j - 1);
            const double* mid   = grid.row(j);
            double*       out   = next.row(j);

            for (std::size_t i = 1; i < nx - 1; ++i) {
                out[i] = 0.25 * (mid[i - 1] + mid[i + 1] + below[i] + above[i]);

                // Residual accumulated inline — no second pass needed.
                const double diff = std::abs(out[i] - mid[i]);
                residual = std::max(residual, diff);
            }
        }

        bc.apply(next);
        std::swap(grid, next);
        ++iterations;
    }

    return SolverResult{.iterations = iterations,
                        .residual   = residual,
                        .converged  = residual <= tolerance};
}

} // namespace laplace
