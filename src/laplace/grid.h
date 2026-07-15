#pragma once

/**
 * @file grid.h
 * @brief Declares the Grid class: a 2-D scalar field with a guaranteed
 *        memory layout suitable for HPC solvers.
 */

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace laplace {

/**
 * @brief A uniform 2-D grid storing one double-precision value per cell.
 *
 * ## Memory layout
 *
 * Data is stored **row-major**: element `(i, j)` — column @p i, row @p j —
 * is located at
 * @code
 *   data()[j * nx() + i]
 * @endcode
 * Consecutive values in the x-direction (varying @p i, fixed @p j) are
 * **contiguous in memory**.  The solver's innermost loop should therefore
 * iterate over @p i to maximise cache efficiency and enable
 * auto-vectorisation.  Use row() to obtain a raw pointer to the start of
 * a row and pass it directly to MPI send/receive calls or CUDA kernels.
 *
 * ## Class invariants
 *
 * The following hold at the end of every constructor and after every
 * member-function call:
 *   - `nx() > 0`
 *   - `ny() > 0`
 *   - `data()` points to a contiguous array of exactly `nx() * ny()` doubles
 *
 * These invariants are established in the constructor (which throws on
 * invalid arguments) and are never violated by any other member function.
 *
 * ## Bounds checking
 *
 * `operator()` and `row()` assert their preconditions in **Debug** builds
 * (i.e., when `NDEBUG` is *not* defined).  In **Release** builds the
 * assertions are compiled away and the functions are zero-overhead wrappers
 * around a single array index.
 */
class Grid {
public:
    // ── Construction ─────────────────────────────────────────────────────────

    /**
     * @brief Construct a grid of size @p nx × @p ny.
     *
     * @param nx            Number of columns (x-direction). Must be > 0.
     * @param ny            Number of rows    (y-direction). Must be > 0.
     * @param initial_value Value written to every cell on construction.
     *                      Default: 0.0.
     * @throws std::invalid_argument if @p nx == 0 or @p ny == 0.
     */
    Grid(std::size_t nx, std::size_t ny, double initial_value = 0.0);

    // ── Element access ────────────────────────────────────────────────────────

    /**
     * @brief Read/write access to element at column @p i, row @p j.
     * @pre `i < nx()` and `j < ny()` (asserted in Debug builds).
     */
    double& operator()(std::size_t i, std::size_t j) noexcept {
        assert(i < nx_ && j < ny_);
        return data_[j * nx_ + i];
    }

    /**
     * @brief Read-only access to element at column @p i, row @p j.
     * @pre `i < nx()` and `j < ny()` (asserted in Debug builds).
     */
    double operator()(std::size_t i, std::size_t j) const noexcept {
        assert(i < nx_ && j < ny_);
        return data_[j * nx_ + i];
    }

    // ── Raw data access ───────────────────────────────────────────────────────
    //
    // These are the primary interface for performance-critical solver code.
    // The layout contract is part of the class interface, not an
    // implementation detail: element (i, j) is at data()[j * nx() + i].

    /**
     * @brief Pointer to the first element of the flat data array.
     *
     * Element `(i, j)` is at `data()[j * nx() + i]`.
     * This pointer remains valid for the lifetime of the Grid object (or
     * until the object is move-assigned).
     */
    double*       data() noexcept       { return data_.data(); }

    /// @copydoc data()
    const double* data() const noexcept { return data_.data(); }

    /**
     * @brief Pointer to the first element of row @p j.
     *
     * Equivalent to `data() + j * nx()`.  Use this to pass a row slice
     * directly to an MPI send/receive call or a CUDA kernel without
     * spelling out the stride at every call site.
     *
     * @pre `j < ny()` (asserted in Debug builds).
     */
    double* row(std::size_t j) noexcept {
        assert(j < ny_);
        return data_.data() + j * nx_;
    }

    /// @copydoc row(std::size_t)
    const double* row(std::size_t j) const noexcept {
        assert(j < ny_);
        return data_.data() + j * nx_;
    }

    // ── Dimensions ────────────────────────────────────────────────────────────

    /// Number of columns (x-direction).
    std::size_t nx() const noexcept { return nx_; }

    /// Number of rows (y-direction).
    std::size_t ny() const noexcept { return ny_; }

    // ── I/O ───────────────────────────────────────────────────────────────────

    /**
     * @brief Write the grid to a plain-text file.
     *
     * Output format: one row per line, values separated by spaces, rows
     * written from @p j = 0 (bottom) to @p j = ny-1 (top).  The file can
     * be loaded directly with `numpy.loadtxt()` or gnuplot's `matrix` mode.
     *
     * @param filename Path to the output file.
     * @throws std::runtime_error if the file cannot be opened for writing.
     */
    void write(const std::string& filename) const;

private:
    std::size_t         nx_;   ///< Number of columns.
    std::size_t         ny_;   ///< Number of rows.
    std::vector<double> data_; ///< Flat storage, row-major.
};

} // namespace laplace
