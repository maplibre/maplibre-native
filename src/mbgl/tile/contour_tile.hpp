#pragma once

#include <mbgl/tile/geometry_tile.hpp>

namespace mbgl {

class RasterDEMTile;
class TileParameters;

// A vector-tile that renders contour-line features for a single (z, x, y).
//
// Spike: emits one hard-coded horizontal line through the middle of the tile
// when populated. The point of the spike is to prove the source-on-source
// rendering pipeline works end-to-end before Task 4 plugs in the real
// marching-squares algorithm. Real implementation reads the upstream DEM
// tile's DEMData and produces contour line features.
class ContourTile final : public GeometryTile {
public:
    ContourTile(const OverscaledTileID&, std::string sourceID, const TileParameters&, TileObserver* = nullptr);

    // Populate the tile from an upstream DEM tile that just finished parsing.
    // The contour source's tile-load listener calls this on each
    // RenderContourSource pyramid tile that matches an arriving DEM tile.
    // Until this is called, the tile is non-renderable (empty bucket).
    void populateFromDEM(const RasterDEMTile& demTile);
};

} // namespace mbgl
