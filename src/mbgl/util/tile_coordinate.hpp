#pragma once

#include <mbgl/map/transform_state.hpp>
#include <mbgl/math/clamp.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/util/projection.hpp>

namespace mbgl {

using TileCoordinatePoint = Point<double>;

// Has floating point x/y coordinates.
// Used for computing the tiles that need to be visible in the viewport.
// In mapbox-gl-js, this is named MercatorCoordinate.
class TileCoordinate {
public:
    TileCoordinatePoint p;
    double z;

    static TileCoordinate fromLatLng(double zoom, const LatLng& latLng) {
        const double scale = std::pow(2.0, zoom);
        return {.p = Projection::project(latLng, scale) / util::tileSize_D, .z = zoom};
    }

    static TileCoordinate fromScreenCoordinate(const TransformState& state,
                                               uint8_t zoom,
                                               const ScreenCoordinate& screenCoordinate) {
        return state.screenCoordinateToTileCoordinate(screenCoordinate, zoom);
    }

    TileCoordinate zoomTo(double zoom) const {
        const double scaleDiff = std::pow(2.0, zoom - z);
        return {.p = p * scaleDiff, .z = zoom};
    }

    static GeometryCoordinate toGeometryCoordinate(const UnwrappedTileID& tileID, const TileCoordinatePoint& point) {
        const double scale = std::pow(2.0, tileID.canonical.z);
        auto zoomed = TileCoordinate{.p = point, .z = 0}.zoomTo(tileID.canonical.z);
        return {int16_t(util::clamp<int64_t>(
                    static_cast<int64_t>((zoomed.p.x - tileID.canonical.x - tileID.wrap * scale) * util::EXTENT),
                    std::numeric_limits<int16_t>::min(),
                    std::numeric_limits<int16_t>::max())),
                int16_t(util::clamp<int64_t>(static_cast<int64_t>((zoomed.p.y - tileID.canonical.y) * util::EXTENT),
                                             std::numeric_limits<int16_t>::min(),
                                             std::numeric_limits<int16_t>::max()))};
    }
};

} // namespace mbgl
