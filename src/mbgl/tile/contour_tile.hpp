#pragma once

#include <mbgl/algorithm/contour/units.hpp>
#include <mbgl/tile/geometry_tile.hpp>

#include <mapbox/std/weak.hpp>

#include <cstdint>
#include <vector>

namespace mbgl {

class RasterDEMTile;
class TileParameters;

// A vector-tile that renders contour-line features for a single (z, x, y).
//
// `populateFromDEM` snapshots the buffered (dim + 2·border) DEM image on
// the render thread, dispatches marching-squares + smoothing to the
// background scheduler, and on reply (render thread) converts the
// resulting line segments into MVT features and feeds them to the
// standard `GeometryTile::setData` path. A `WeakPtrFactory` guards
// against the tile being destroyed (panned away) before the background
// work completes.
//
// Tile-edge continuity is preserved by sampling DEMData's neighbour-
// border (populated by `RenderRasterDEMSource` via `backfillBorder` as
// adjacent DEM tiles arrive). Lines crossing the tile edge are clipped
// against the tile bbox in `toFeatures` so adjacent tiles' smoothed
// contours meet exactly at the seam.
class ContourTile final : public GeometryTile {
public:
    ContourTile(const OverscaledTileID&, std::string sourceID, const TileParameters&, TileObserver* = nullptr);

    // Populate the tile from an upstream DEM tile that just finished parsing.
    // `intervalDisplayUnits` is the contour spacing in the source's display
    // unit (e.g. 100 feet, 50 metres) — the source's `intervals` schedule is
    // resolved to a single value per the tile's zoom by the caller.
    // `majorMultiplier` is the source's `majorMultiplier` schedule resolved
    // to a single positive integer per the tile's zoom by the caller; lines
    // at elevations divisible by `intervalDisplayUnits × majorMultiplier`
    // get `major: true`. `unit` controls the metres↔display conversion.
    void populateFromDEM(const RasterDEMTile& demTile,
                         double intervalDisplayUnits,
                         std::int64_t majorMultiplier,
                         const algorithm::contour::UnitConfig& unit);

private:
    mapbox::base::WeakPtrFactory<ContourTile> weakFactory{this};
    // Do not add members here, see `WeakPtrFactory`.
};

} // namespace mbgl
