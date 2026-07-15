#include "laplace/grid.h"
#include "laplace/boundary_conditions.h"
#include "laplace/solver.h"
#include <gtest/gtest.h>

// Smoke tests verifying Grid invariants and the memory layout contract.
// Real solver tests (convergence, exact solutions, symmetry) are added next.

namespace laplace {
namespace test {

TEST(GridTest, InvalidDimensionsThrow)
{
    EXPECT_THROW(Grid(0, 10), std::invalid_argument);
    EXPECT_THROW(Grid(10, 0), std::invalid_argument);
}

TEST(GridTest, DimensionsAndLayoutContract)
{
    Grid g(4, 3, 0.0);
    EXPECT_EQ(g.nx(), 4u);
    EXPECT_EQ(g.ny(), 3u);

    // Verify the documented layout: element (i,j) is at data()[j*nx+i].
    g(2, 1) = 42.0;
    EXPECT_DOUBLE_EQ(g.data()[1 * 4 + 2], 42.0);

    // Verify row() is consistent with data().
    EXPECT_EQ(g.row(1), g.data() + 1 * 4);
}

} // namespace test
} // namespace laplace
