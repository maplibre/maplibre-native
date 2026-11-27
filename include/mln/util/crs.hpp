#pragma once

#include <mln/util/tile_matrix_set.hpp>

#include <array>
#include <functional>
#include <span>
#include <string>

namespace mln {

/// A planar coordinate reference system and the square, power-of-two quad tile grid laid over it, so a map can draw
/// tiles that were cut in that system. Register it with `addProjection` and select it by name with the style's
/// `projection.type`. Every tile source of the map is taken to be cut in `tileMatrixSet`, the tiles are drawn as they
/// are, and CRS units count as meters wherever the map converts meters.
/// This API is experimental and may change in a future release.
struct CRS {
    /// The name `projection.type` takes, like `EPSG:2193`. It must contain a colon: names without one are reserved for
    /// built-in projections.
    std::string name;
    /// Each lng/lat in degrees to CRS coordinates, NaN where it has none. Called from any thread, several at once.
    std::function<void(std::span<const std::array<double, 2>> lngLats, std::span<std::array<double, 2>> positions)>
        forward;
    /// Each CRS position to lng/lat in degrees, NaN where there is no answer. Called from any thread, several at once.
    /// Both converters are destroyed on whichever thread drops the last reference to the projection.
    /// Neither converter may throw: NaN marks a position without an answer.
    std::function<void(std::span<const std::array<double, 2>> positions, std::span<std::array<double, 2>> lngLats)>
        inverse;
    TileMatrixSet tileMatrixSet;
};

/// Registers a projection for every map. Throws `std::invalid_argument` if the name has no colon or is taken, if
/// `tileMatrixSet.extentAtZoom0` is not positive, or if `forward` or `inverse` is empty.
/// This API is experimental and may change in a future release.
void addProjection(CRS);

/// Maps already in the projection keep it until their projection changes. Throws `std::invalid_argument` for a name
/// without a colon, since built-in projections cannot be removed.
/// This API is experimental and may change in a future release.
void removeProjection(const std::string& name);

} // namespace mln
