#include <mbgl/renderer/dem_elevation_provider.hpp>

#include <mbgl/geometry/dem_data.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/tile/raster_dem_tile.hpp>

namespace mbgl {

DEMElevationProvider::DEMElevationProvider(const RenderSource* demSource_, double exaggeration_)
    : demSource(demSource_),
      exaggeration(exaggeration_) {}

std::optional<Range<double>> DEMElevationProvider::getTileElevationRange(const CanonicalTileID& id) const {
    if (!demSource) {
        return std::nullopt;
    }

    const auto renderTiles = demSource->getRawRenderTiles();
    if (renderTiles->empty()) {
        return std::nullopt;
    }

    // The tile's own DEM, or failing that the deepest loaded ancestor: an ancestor's
    // range covers this tile's area, so it stays conservative, just looser.
    const DEMData* best = nullptr;
    uint8_t bestZoom = 0;
    for (const auto& renderTile : *renderTiles) {
        const auto& tile = renderTile.getTile();
        if (tile.kind != Tile::Kind::RasterDEM) {
            continue;
        }
        const auto& candidate = renderTile.id.canonical;
        const bool covers = candidate == id || id.isChildOf(candidate);
        if (!covers || (best && candidate.z <= bestZoom)) {
            continue;
        }
        const auto* demTile = static_cast<const RasterDEMTile*>(&tile);
        const auto* bucket = const_cast<RasterDEMTile*>(demTile)->getBucket();
        if (!bucket) {
            continue;
        }
        best = &bucket->getDEMData();
        bestZoom = candidate.z;
        if (candidate == id) {
            break; // exact match; nothing looser can improve on it
        }
    }

    if (!best) {
        return std::nullopt;
    }

    // Exaggeration is applied to the mesh in the terrain vertex shader, so the bounds
    // have to carry it too, or an exaggerated peak would still be culled.
    return Range<double>{best->getMinElevation() * exaggeration, best->getMaxElevation() * exaggeration};
}

} // namespace mbgl
