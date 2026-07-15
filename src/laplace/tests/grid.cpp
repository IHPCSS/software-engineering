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

namespace laplace {
namespace test {

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
    Grid g(4, 3, kFill);

    EXPECT_EQ(g.nx(), 4u);
    EXPECT_EQ(g.ny(), 3u);

    // Every cell must hold the initial value.
    for (std::size_t j = 0; j < g.ny(); ++j)
        for (std::size_t i = 0; i < g.nx(); ++i)
            EXPECT_DOUBLE_EQ(g(i, j), kFill) << "  at (" << i << ", " << j << ")";
}

// ── Mutable element access ────────────────────────────────────────────────────

TEST(GridTest, MutableOperatorParens)
{
    Grid g(4, 3, 0.0);
    g(2, 1) = 42.0;
    EXPECT_DOUBLE_EQ(g(2, 1), 42.0);
}

// ── Const element access ──────────────────────────────────────────────────────
//
// The const overloads of operator(), data(), and row() are only reachable
// through a const reference.  Each test below binds one.

TEST(GridTest, ConstOperatorParens)
{
    Grid g(4, 3, 5.0);
    const Grid& cg = g;
    EXPECT_DOUBLE_EQ(cg(1, 2), 5.0);
}

// ── Memory layout contract ────────────────────────────────────────────────────
//
// The documented invariant: element (i, j) lives at data()[j * nx + i].
// row(j) is guaranteed to equal data() + j * nx.
// These contracts are the primary extension points for MPI and CUDA code.

TEST(GridTest, DataLayoutContract)
{
    Grid g(4, 3, 0.0);
    g(2, 1) = 42.0;
    // Mutable data() accessor.
    EXPECT_DOUBLE_EQ(g.data()[1 * 4 + 2], 42.0);
}

TEST(GridTest, RowPointerContract)
{
    Grid g(4, 3, 0.0);
    // Mutable row() accessor.
    EXPECT_EQ(g.row(1), g.data() + 1 * 4);
}

TEST(GridTest, ConstDataAndRow)
{
    Grid g(4, 3, 9.0);
    const Grid& cg = g;

    // Const data() must point to the same storage as the mutable version.
    EXPECT_EQ(cg.data(), g.data());
    // Const row() must satisfy the same layout contract.
    EXPECT_EQ(cg.row(2), cg.data() + 2 * 4);
    // Values must be readable through the const pointer.
    EXPECT_DOUBLE_EQ(cg.data()[0], 9.0);
}

// ── write() ───────────────────────────────────────────────────────────────────

TEST(GridTest, WriteAndReadBack)
{
    // 3 × 2 grid so we can check exact values and exercise the
    // space-between-values branch (written between columns, not after the last).
    //
    //   j=1:  4  5  6
    //   j=0:  1  2  3
    Grid g(3, 2, 0.0);
    g(0, 0) = 1.0;  g(1, 0) = 2.0;  g(2, 0) = 3.0;
    g(0, 1) = 4.0;  g(1, 1) = 5.0;  g(2, 1) = 6.0;

    const std::string path = "/tmp/test_grid_write.txt";
    g.write(path);

    // Read the values back and verify order and content.
    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open()) << "Could not open " << path;

    double v = 0.0;
    // Row j=0 written first.
    ifs >> v; EXPECT_DOUBLE_EQ(v, 1.0);
    ifs >> v; EXPECT_DOUBLE_EQ(v, 2.0);
    ifs >> v; EXPECT_DOUBLE_EQ(v, 3.0);
    // Row j=1 written second.
    ifs >> v; EXPECT_DOUBLE_EQ(v, 4.0);
    ifs >> v; EXPECT_DOUBLE_EQ(v, 5.0);
    ifs >> v; EXPECT_DOUBLE_EQ(v, 6.0);
}

TEST(GridTest, WriteThrowsOnBadPath)
{
    Grid g(2, 2, 0.0);
    EXPECT_THROW(g.write("/no/such/directory/file.txt"), std::runtime_error);
}

} // namespace test
} // namespace laplace
