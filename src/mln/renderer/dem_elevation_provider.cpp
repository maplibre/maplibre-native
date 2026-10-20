#include <mln/renderer/dem_elevation_provider.hpp>

#include <mln/geometry/dem_data.hpp>
#include <mln/renderer/buckets/hillshade_bucket.hpp>
#include <mln/renderer/render_source.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/tile/raster_dem_tile.hpp>

#include <algorithm>
#include <limits>

namespace mln {

DEMElevationProvider::DEMElevationProvider(const RenderSource* demSource_, double exaggeration_)
    : demSource(demSource_),
      exaggeration(exaggeration_) {
    // Precompute the aggregate elevation range of every loaded DEM tile (once, here,
    // not per query). It is the fallback for tiles with no loaded DEM, so the cover
    // dilation can re-test a not-yet-loaded frontier neighbour as if it were about as
    // tall/deep as the terrain already in view.
    if (!demSource) {
        return;
    }
    const auto renderTiles = demSource->getRawRenderTiles();
    double minEle = std::numeric_limits<double>::max();
    double maxEle = std::numeric_limits<double>::lowest();
    for (const auto& renderTile : *renderTiles) {
        const auto& tile = renderTile.getTile();
        if (tile.kind != Tile::Kind::RasterDEM) {
            continue;
        }
        const auto* demTile = static_cast<const RasterDEMTile*>(&tile);
        const auto* bucket = const_cast<RasterDEMTile*>(demTile)->getBucket();
        if (!bucket) {
            continue;
        }
        const auto& demData = bucket->getDEMData();
        minEle = std::min(minEle, static_cast<double>(demData.getMinElevation()));
        maxEle = std::max(maxEle, static_cast<double>(demData.getMaxElevation()));
    }
    if (minEle <= maxEle) {
        loadedRange = Range<double>{minEle * exaggeration, maxEle * exaggeration};
    }
}

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
        // No DEM covers this tile. Fall back to the range of terrain loaded in view
        // (nullopt only if nothing is loaded), so the cover dilation's frustumCull can
        // decide the tile on its (assumed) elevation instead of treating it as flat.
        return loadedRange;
    }

    // Exaggeration is applied to the mesh in the terrain vertex shader, so the bounds
    // have to carry it too, or an exaggerated peak would still be culled.
    return Range<double>{best->getMinElevation() * exaggeration, best->getMaxElevation() * exaggeration};
}

} // namespace mln
