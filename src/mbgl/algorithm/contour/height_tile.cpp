#include <mbgl/algorithm/contour/height_tile.hpp>

#include <cstddef>
#include <limits>

namespace mbgl {
namespace algorithm {
namespace contour {

double sample(const HeightTile& tile, int x, int y) {
    if (x < 0 || y < 0 || x >= tile.width || y >= tile.height) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return static_cast<double>(tile.samples[static_cast<std::size_t>(y) * tile.width + x]);
}

} // namespace contour
} // namespace algorithm
} // namespace mbgl
