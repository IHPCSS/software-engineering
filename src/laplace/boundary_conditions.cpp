#include "laplace/boundary_conditions.h"

namespace laplace {

// ── ConstantBC ────────────────────────────────────────────────────────────────

void ConstantBC::apply(Grid& grid) const
{
    const std::size_t nx = grid.nx();
    const std::size_t ny = grid.ny();

    // Left and right columns first (all rows, including corners).
    for (std::size_t j = 0; j < ny; ++j) {
        grid(0,      j) = left_;
        grid(nx - 1, j) = right_;
    }
    // Bottom and top rows written last — they own the four corner cells.
    for (std::size_t i = 0; i < nx; ++i) {
        grid(i, 0)      = bottom_;
        grid(i, ny - 1) = top_;
    }
}

// ── CornerHeatBC ──────────────────────────────────────────────────────────────

void CornerHeatBC::apply(Grid& grid) const
{
    const std::size_t nx = grid.nx();
    const std::size_t ny = grid.ny();

    // Left edge: 0 (all rows).
    for (std::size_t j = 0; j < ny; ++j) {
        grid(0, j) = 0.0;
    }

    // Right edge: linear ramp from 0 (top) to 100 (bottom).
    // Note: j=0 is the bottom row in our convention.
    for (std::size_t j = 0; j < ny; ++j) {
        grid(nx - 1, j) = (100.0 / static_cast<double>(ny - 1)) * static_cast<double>(j);
    }

    // Top edge: 0 (all columns, written last so it owns the top corners).
    for (std::size_t i = 0; i < nx; ++i) {
        grid(i, ny - 1) = 0.0;
    }

    // Bottom edge: linear ramp from 0 (left) to 100 (right).
    // Written last so it owns the bottom corners.
    for (std::size_t i = 0; i < nx; ++i) {
        grid(i, 0) = (100.0 / static_cast<double>(nx - 1)) * static_cast<double>(i);
    }
}

} // namespace laplace
