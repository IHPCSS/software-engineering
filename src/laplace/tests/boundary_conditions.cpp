/**
 * @file test_boundary_conditions.cpp
 * @brief Unit tests for ConstantBC and CornerHeatBC.
 *
 * Coverage targets
 * ================
 * boundary_conditions.cpp
 *   - ConstantBC::apply()    : left/right loop, bottom/top loop, corner
 *                              ownership (top/bottom override the corners set
 *                              by left/right)
 *   - CornerHeatBC::apply()  : left loop (=0), right-ramp loop, top loop (=0),
 *                              bottom-ramp loop, corner values for a 3×3 grid
 *
 * boundary_conditions.h (inline / defaulted)
 *   - ConstantBC constructor
 *   - BoundaryConditions virtual destructor (exercised when objects go out
 *     of scope via a base-class pointer in CornerHeatBC polymorphism test)
 */

#include "laplace/boundary_conditions.h"
#include "laplace/grid.h"

#include <gtest/gtest.h>
#include <memory>

namespace laplace {
namespace test {

// ── ConstantBC ────────────────────────────────────────────────────────────────

// Use a 5×4 grid (nx=5, ny=4) to give non-trivial interior rows/cols.
//   top=10, bottom=20, left=1, right=2

TEST(ConstantBCTest, LeftAndRightEdges)
{
    Grid g(5, 4, 99.0);
    ConstantBC bc(/*top*/10.0, /*bottom*/20.0, /*left*/1.0, /*right*/2.0);
    bc.apply(g);

    // Left/right columns are written first and cover *all* rows including the
    // corners.  The bottom/top loops are written afterwards and will override
    // the corner cells — see CornerOwnership test.  So we check only the
    // non-corner interior rows here.
    for (std::size_t j = 1; j < 3; ++j) {
        EXPECT_DOUBLE_EQ(g(0,      j), 1.0) << "left  j=" << j;
        EXPECT_DOUBLE_EQ(g(4,      j), 2.0) << "right j=" << j;
    }
}

TEST(ConstantBCTest, BottomAndTopEdges)
{
    Grid g(5, 4, 99.0);
    ConstantBC bc(/*top*/10.0, /*bottom*/20.0, /*left*/1.0, /*right*/2.0);
    bc.apply(g);

    for (std::size_t i = 0; i < 5; ++i) {
        EXPECT_DOUBLE_EQ(g(i, 0), 20.0) << "bottom i=" << i;
        EXPECT_DOUBLE_EQ(g(i, 3), 10.0) << "top    i=" << i;
    }
}

TEST(ConstantBCTest, CornersOwnedByTopAndBottom)
{
    // Top/bottom rows are written *after* left/right columns.
    // So all four corner cells belong to top or bottom, not left or right.
    //   top=10, bottom=20, left=1, right=2 (different from all corners)
    Grid g(5, 4, 99.0);
    ConstantBC bc(/*top*/10.0, /*bottom*/20.0, /*left*/1.0, /*right*/2.0);
    bc.apply(g);

    EXPECT_DOUBLE_EQ(g(0, 0), 20.0); // bottom-left corner  → bottom
    EXPECT_DOUBLE_EQ(g(4, 0), 20.0); // bottom-right corner → bottom
    EXPECT_DOUBLE_EQ(g(0, 3), 10.0); // top-left corner     → top
    EXPECT_DOUBLE_EQ(g(4, 3), 10.0); // top-right corner    → top
}

TEST(ConstantBCTest, UniformBC)
{
    // Edge case: all four values the same.  Every perimeter cell should be 5.
    Grid g(4, 4, 0.0);
    ConstantBC bc(5.0, 5.0, 5.0, 5.0);
    bc.apply(g);

    // Check all perimeter cells (rows 0, ny-1 and cols 0, nx-1).
    for (std::size_t i = 0; i < 4; ++i) {
        EXPECT_DOUBLE_EQ(g(i, 0), 5.0);
        EXPECT_DOUBLE_EQ(g(i, 3), 5.0);
    }
    for (std::size_t j = 0; j < 4; ++j) {
        EXPECT_DOUBLE_EQ(g(0, j), 5.0);
        EXPECT_DOUBLE_EQ(g(3, j), 5.0);
    }
    // Interior cells must not be touched.
    EXPECT_DOUBLE_EQ(g(1, 1), 0.0);
    EXPECT_DOUBLE_EQ(g(2, 2), 0.0);
}

// ── CornerHeatBC ──────────────────────────────────────────────────────────────
//
// Use a 3×3 grid (nx=3, ny=3) so every value can be computed by hand.
//
// Apply order:
//   1. Left  column   : all rows → 0
//   2. Right column   : j=0→0, j=1→50, j=2→100  (then overridden below)
//   3. Top   row      : all cols → 0           (overrides right-top corner)
//   4. Bottom row     : i=0→0, i=1→50, i=2→100 (overrides right-bottom corner)
//
// Final expected values:
//   g(0,0)=  0   g(1,0)= 50   g(2,0)=100   ← bottom row ramp
//   g(0,1)=  0   g(1,1)= 99   g(2,1)= 50   ← left=0, interior untouched, right ramp
//   g(0,2)=  0   g(1,2)=  0   g(2,2)=  0   ← top row (overrides right-top)

TEST(CornerHeatBCTest, LeftEdgeIsZero)
{
    Grid g(3, 3, 99.0);
    CornerHeatBC bc;
    bc.apply(g);

    for (std::size_t j = 0; j < 3; ++j)
        EXPECT_DOUBLE_EQ(g(0, j), 0.0) << "left j=" << j;
}

TEST(CornerHeatBCTest, TopEdgeIsZero)
{
    Grid g(3, 3, 99.0);
    CornerHeatBC bc;
    bc.apply(g);

    for (std::size_t i = 0; i < 3; ++i)
        EXPECT_DOUBLE_EQ(g(i, 2), 0.0) << "top i=" << i;
}

TEST(CornerHeatBCTest, BottomEdgeRamp)
{
    Grid g(3, 3, 99.0);
    CornerHeatBC bc;
    bc.apply(g);

    // Bottom row: ramp 0 → 100 over nx-1=2 steps.
    EXPECT_DOUBLE_EQ(g(0, 0),   0.0);
    EXPECT_DOUBLE_EQ(g(1, 0),  50.0);
    EXPECT_DOUBLE_EQ(g(2, 0), 100.0);
}

TEST(CornerHeatBCTest, RightEdgeInteriorRamp)
{
    Grid g(3, 3, 99.0);
    CornerHeatBC bc;
    bc.apply(g);

    // Right column interior rows (j=1 only for 3×3).
    // j=1: (100 / (ny-1=2)) * 1 = 50.
    EXPECT_DOUBLE_EQ(g(2, 1), 50.0);
}

TEST(CornerHeatBCTest, TopRightCornerOwnedByTop)
{
    // Right-ramp would give (nx-1, ny-1) = 100, but top loop runs after and
    // resets it to 0.
    Grid g(3, 3, 99.0);
    CornerHeatBC bc;
    bc.apply(g);

    EXPECT_DOUBLE_EQ(g(2, 2), 0.0);
}

TEST(CornerHeatBCTest, BottomRightCornerOwnedByBottom)
{
    // Right-ramp gives (nx-1, 0) = 0, but bottom loop runs last and sets
    // i=nx-1 → 100.
    Grid g(3, 3, 99.0);
    CornerHeatBC bc;
    bc.apply(g);

    EXPECT_DOUBLE_EQ(g(2, 0), 100.0);
}

TEST(CornerHeatBCTest, InteriorUntouched)
{
    Grid g(3, 3, 99.0);
    CornerHeatBC bc;
    bc.apply(g);

    // Only (1,1) is interior in a 3×3 grid.
    EXPECT_DOUBLE_EQ(g(1, 1), 99.0);
}

// ── Polymorphism / virtual destructor ─────────────────────────────────────────
//
// Deleting a derived object through a base-class pointer exercises the virtual
// destructor in BoundaryConditions.

TEST(BoundaryConditionsTest, VirtualDestructorViaBasePointer)
{
    // No EXPECT_ needed: the test passes if it doesn't crash or leak.
    std::unique_ptr<BoundaryConditions> bc =
        std::make_unique<ConstantBC>(1.0, 2.0, 3.0, 4.0);
    // Destructor runs here when unique_ptr goes out of scope.
}

} // namespace test
} // namespace laplace
