/**
 * @file test_grid.cpp
 * @brief Unit tests for the Grid class.
 *
 * Coverage targets
 * ================
 * grid.cpp
 *   - Grid::Grid()          : valid construction, throw on nx=0, throw on ny=0
 *   - Grid::write()         : success path (verifying written content), bad-path
 *                             throw, both branches of the space-between-values
 *                             conditional (i+1 < nx  →  true and false)
 *
 * grid.h  (inline functions — covered by instantiation)
 *   - operator()(i,j)       : mutable and const overloads
 *   - data()                : mutable and const overloads
 *   - row(j)                : mutable and const overloads
 *   - nx(), ny()
 */

#include "laplace/grid.h"

#include <fstream>
#include <gtest/gtest.h>

namespace laplace::test {

// ── Construction ──────────────────────────────────────────────────────────────

TEST(GridTest, InvalidNxThrows)
{
    EXPECT_THROW(Grid(0, 5), std::invalid_argument);
}

TEST(GridTest, InvalidNyThrows)
{
    EXPECT_THROW(Grid(5, 0), std::invalid_argument);
}

TEST(GridTest, ConstructionAndInitialValue)
{
    constexpr double kFill = 7.0;
    Grid grid(4, 3, kFill);

    EXPECT_EQ(grid.nx(), 4U);
    EXPECT_EQ(grid.ny(), 3U);

    // Every cell must hold the initial value.
    for (std::size_t j = 0; j < grid.ny(); ++j) {
        for (std::size_t i = 0; i < grid.nx(); ++i) {
            EXPECT_DOUBLE_EQ(grid(i, j), kFill) << "  at (" << i << ", " << j << ")";
        }
    }
}

// ── Mutable element access ────────────────────────────────────────────────────

TEST(GridTest, MutableOperatorParens)
{
    Grid grid(4, 3, 0.0);
    grid(2, 1) = 42.0;
    EXPECT_DOUBLE_EQ(grid(2, 1), 42.0);
}

// ── Const element access ──────────────────────────────────────────────────────
//
// The const overloads of operator(), data(), and row() are only reachable
// through a const reference.  Each test below binds one.

TEST(GridTest, ConstOperatorParens)
{
    Grid grid(4, 3, 5.0);
    const Grid& const_grid = grid;
    EXPECT_DOUBLE_EQ(const_grid(1, 2), 5.0);
}

// ── Memory layout contract ────────────────────────────────────────────────────
//
// The documented invariant: element (i, j) lives at data()[j * nx + i].
// row(j) is guaranteed to equal data() + j * nx.
// These contracts are the primary extension points for MPI and CUDA code.

TEST(GridTest, DataLayoutContract)
{
    Grid grid(4, 3, 0.0);
    grid(2, 1) = 42.0;
    // Mutable data() accessor: element (col=2, row=1) is at data()[1*4 + 2].
    EXPECT_DOUBLE_EQ(grid.data()[(std::size_t{1} * 4) + 2], 42.0);
}

TEST(GridTest, RowPointerContract)
{
    Grid grid(4, 3, 0.0);
    // Mutable row() accessor: row(1) must equal data() + 1*nx.
    EXPECT_EQ(grid.row(1), grid.data() + (std::size_t{1} * 4));
}

TEST(GridTest, ConstDataAndRow)
{
    Grid grid(4, 3, 9.0);
    const Grid& const_grid = grid;

    // Const data() must point to the same storage as the mutable version.
    EXPECT_EQ(const_grid.data(), grid.data());
    // Const row() must satisfy the same layout contract.
    EXPECT_EQ(const_grid.row(2), const_grid.data() + (std::size_t{2} * 4));
    // Values must be readable through the const pointer.
    EXPECT_DOUBLE_EQ(const_grid.data()[0], 9.0);
}

// ── write() ───────────────────────────────────────────────────────────────────

TEST(GridTest, WriteAndReadBack)
{
    // 3 × 2 grid so we can check exact values and exercise the
    // space-between-values branch (written between columns, not after the last).
    //
    //   j=1:  4  5  6
    //   j=0:  1  2  3
    Grid grid(3, 2, 0.0);
    grid(0, 0) = 1.0;  grid(1, 0) = 2.0;  grid(2, 0) = 3.0;
    grid(0, 1) = 4.0;  grid(1, 1) = 5.0;  grid(2, 1) = 6.0;

    const std::string path = "/tmp/test_grid_write.txt";
    grid.write(path);

    // Read the values back and verify order and content.
    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open()) << "Could not open " << path;

    double val = 0.0;
    // Row j=0 written first.
    ifs >> val; EXPECT_DOUBLE_EQ(val, 1.0);
    ifs >> val; EXPECT_DOUBLE_EQ(val, 2.0);
    ifs >> val; EXPECT_DOUBLE_EQ(val, 3.0);
    // Row j=1 written second.
    ifs >> val; EXPECT_DOUBLE_EQ(val, 4.0);
    ifs >> val; EXPECT_DOUBLE_EQ(val, 5.0);
    ifs >> val; EXPECT_DOUBLE_EQ(val, 6.0);
}

TEST(GridTest, WriteThrowsOnBadPath)
{
    Grid grid(2, 2, 0.0);
    EXPECT_THROW(grid.write("/no/such/directory/file.txt"), std::runtime_error);
}

} // namespace laplace::test
