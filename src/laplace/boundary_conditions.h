#pragma once

/**
 * @file boundary_conditions.h
 * @brief Abstract boundary-condition interface and two concrete implementations
 *        for the 2-D Laplace equation.
 */

#include "laplace/grid.h"

namespace laplace {

// ── Abstract interface ────────────────────────────────────────────────────────

/**
 * @brief Abstract interface for Dirichlet boundary conditions.
 *
 * The Solver calls apply() after every Jacobi sweep to re-impose boundary
 * values on the perimeter cells of the grid.  Deriving from this class and
 * overriding apply() is the extension point for new physical configurations —
 * the Solver itself never needs to change.
 *
 * ## Design note
 *
 * Only the perimeter cells (the outermost ring of the grid) should be
 * modified by apply().  Interior cells must be left untouched so that the
 * Jacobi sweep remains the sole authority over their values.
 */
class BoundaryConditions {
public:
    virtual ~BoundaryConditions() = default;

    /**
     * @brief Impose boundary values on the perimeter of @p grid.
     *
     * Implementations must write values only to the four edges of @p grid
     * (rows 0 and ny-1, columns 0 and nx-1).  Interior cells must not be
     * modified.
     *
     * @param grid  Grid whose perimeter cells will be overwritten.
     */
    virtual void apply(Grid& grid) const = 0;
};

// ── Concrete: constant values ─────────────────────────────────────────────────

/**
 * @brief Constant Dirichlet conditions: one fixed temperature per edge.
 *
 * Top and bottom rows are written last and therefore own the four corner
 * cells.  Left and right columns set only the non-corner interior rows.
 *
 * Example — "hot top, cold everywhere else":
 * @code
 *   laplace::ConstantBC bc(100.0,0.0,0.0,0.0);
 * @endcode
 */
class ConstantBC : public BoundaryConditions {
public:
    /**
     * @brief Construct with explicit values for all four edges.
     * @param top    Temperature on the top edge    (j = ny-1).
     * @param bottom Temperature on the bottom edge (j = 0).
     * @param left   Temperature on the left edge   (i = 0).
     * @param right  Temperature on the right edge  (i = nx-1).
     */
    ConstantBC(double top, double bottom, double left, double right) noexcept
        : top_(top), bottom_(bottom), left_(left), right_(right) {}

    void apply(Grid& grid) const override;

private:
    double top_;
    double bottom_;
    double left_;
    double right_;
};

// ── Concrete: linear-ramp (original PSC/IHPCSS demo problem) ─────────────────

/**
 * @brief The boundary conditions from the original PSC/IHPCSS Laplace demo.
 *
 * Left and top edges are held at 0 °C.  Right and bottom edges increase
 * linearly from 0 °C to 100 °C, meeting at 100 °C at the bottom-right corner:
 *
 * @verbatim
 *   0    0    0    0    0
 *   +----+----+----+----+
 *   |                   | 100
 *   |                   |
 * 0 |                   | 50
 *   |                   |
 *   |                   | 0
 *   +----+----+----+----+
 *   0   25   50   75  100
 * @endverbatim
 *
 * This produces an asymmetric solution with all the action in the
 * bottom-right corner — a good stress test and a clear visual demonstration
 * of convergence.
 */
class CornerHeatBC : public BoundaryConditions {
public:
    void apply(Grid& grid) const override;
};

} // namespace laplace
