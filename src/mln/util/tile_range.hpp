#pragma once

#include <mln/tile/tile_id.hpp>
#include <mln/util/range.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/projection.hpp>

#include <array>
#include <cmath>

namespace mln {
namespace util {

class TileRange {
public:
    Range<Point<uint32_t>> range;
    Range<uint8_t> zoomRange;

    // Compute the range of tiles covered by the bounds at maxZoom.
    static TileRange fromLatLngBounds(const LatLngBounds& bounds, uint8_t minZoom, uint8_t maxZoom) noexcept {
        if (minZoom > maxZoom) {
            std::swap(minZoom, maxZoom);
        }

        auto swProj = Projection::project(bounds.southwest().wrapped(), maxZoom);
        auto ne = bounds.northeast();
        auto neProj = Projection::project(ne.longitude() > util::LONGITUDE_MAX ? ne.wrapped() : ne, maxZoom);

        const auto maxTile = std::pow(2.0, maxZoom);
        const auto minX = static_cast<uint32_t>(std::floor(swProj.x));
        const auto maxX = static_cast<uint32_t>(std::floor(neProj.x));
        const auto minY = static_cast<uint32_t>(util::clamp(std::floor(neProj.y), 0.0, maxTile));
        const auto maxY = static_cast<uint32_t>(util::clamp(std::floor(swProj.y), 0.0, maxTile));

        return {{{minX, minY}, {maxX, maxY}}, {minZoom, maxZoom}};
    }

    /// The tiles a rectangle in the world square reaches into, as min x, min y, max x, max y in world units, as GL JS's
    /// `TileBounds` counts them.
    static TileRange fromWorldBox(const std::array<double, 4>& box, uint8_t minZoom, uint8_t maxZoom) noexcept {
        if (minZoom > maxZoom) {
            std::swap(minZoom, maxZoom);
        }
        if (box[2] <= 0 || box[3] <= 0 || box[0] >= 1 || box[1] >= 1) {
            // Outside the square: no zoom range contains any tile.
            return {{{0, 0}, {0, 0}}, {1, 0}};
        }
        const double maxTile = std::pow(2.0, maxZoom);
        const auto first = [&](double world) {
            return static_cast<uint32_t>(util::clamp(std::floor(world * maxTile), 0.0, maxTile - 1));
        };
        const auto last = [&](double world) {
            return static_cast<uint32_t>(util::clamp(std::ceil(world * maxTile) - 1, 0.0, maxTile - 1));
        };
        return {{{first(box[0]), first(box[1])}, {last(box[2]), last(box[3])}}, {minZoom, maxZoom}};
    }

    // Compute the range of tiles covered by the bounds.
    static TileRange fromLatLngBounds(const LatLngBounds& bounds, uint8_t z) noexcept {
        return fromLatLngBounds(bounds, z, z);
    }

    bool contains(const CanonicalTileID& tileID) const noexcept {
        if (tileID.z <= zoomRange.max && tileID.z >= zoomRange.min) {
            if (tileID.z == 0) {
                return true;
            }
            uint8_t dz = (zoomRange.max - tileID.z);
            auto x0 = range.min.x >> dz;
            auto x1 = range.max.x >> dz;
            auto y0 = range.min.y >> dz;
            auto y1 = range.max.y >> dz;
            return (range.min.x > range.max.x ? // For wrapped bounds
                        tileID.x >= x0 || tileID.x <= x1
                                              : tileID.x <= x1 && tileID.x >= x0) &&
                   tileID.y <= y1 && tileID.y >= y0;
        }
        return false;
    }

private:
    TileRange(Range<Point<uint32_t>> range_, Range<uint8_t> z_)
        : range(range_),
          zoomRange(z_) {}
};

} // namespace util
} // namespace mln
