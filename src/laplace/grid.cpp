#include "laplace/grid.h"

#include <fstream>
#include <stdexcept>

namespace laplace {

Grid::Grid(std::size_t nx, std::size_t ny, double initial_value)
    : nx_(nx), ny_(ny), data_(nx * ny, initial_value)
{
    if (nx == 0 || ny == 0) {
        throw std::invalid_argument("Grid: dimensions must be > 0");
    }
}

void Grid::write(const std::string& filename) const
{
    std::ofstream ofs(filename);
    if (!ofs) {
        throw std::runtime_error("Grid::write: cannot open '" + filename + "'");
    }
    for (std::size_t j = 0; j < ny_; ++j) {
        const double* row_data = row(j);
        for (std::size_t i = 0; i < nx_; ++i) {
            ofs << row_data[i];
            if (i + 1 < nx_) { ofs << ' '; }
        }
        ofs << '\n';
    }
}

} // namespace laplace
