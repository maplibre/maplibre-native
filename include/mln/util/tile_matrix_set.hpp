#pragma once

#include <mln/util/constants.hpp>

#include <array>
#include <numbers>

namespace mln {

/// The square tile grid over a projection's plane: the top-left corner of tile 0/0/0 (its smallest x and largest y)
/// and the side of that tile, in the projection's units.
struct TileMatrixSet {
    std::array<double, 2> origin{};
    double extentAtZoom0 = 0;
};

/// EPSG:3857's grid in meters, Mercator's: tile 0/0/0 spans half the circumference each way from the origin.
inline constexpr TileMatrixSet mercatorTileMatrixSet = [] {
    constexpr double halfExtent = std::numbers::pi * util::EARTH_RADIUS_M;
    return TileMatrixSet{.origin = {-halfExtent, halfExtent}, .extentAtZoom0 = 2 * halfExtent};
}();

} // namespace mln
