#include <mln/renderer/sources/render_raster_dem_source.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/tile/raster_dem_tile.hpp>
#include <mln/algorithm/update_tile_masks.hpp>
#include <mln/geometry/dem_data.hpp>
#include <mln/renderer/buckets/hillshade_bucket.hpp>
#include <mln/renderer/tile_parameters.hpp>

#include <algorithm>

namespace mln {

using namespace style;

namespace {

Range<int32_t> elevationRangeOf(const HillshadeBucket& bucket) {
    const auto& demData = bucket.getDEMData();
    return {demData.getMinElevation(), demData.getMaxElevation()};
}

} // namespace

RenderRasterDEMSource::RenderRasterDEMSource(Immutable<style::TileSource::Impl> impl_,
                                             const TaggedScheduler& threadPool_)
    : RenderTileSetSource(std::move(impl_), threadPool_) {}

const style::TileSource::Impl& RenderRasterDEMSource::impl() const {
    return static_cast<const style::TileSource::Impl&>(*baseImpl);
}

const std::optional<Tileset>& RenderRasterDEMSource::getTileset() const {
    return impl().tileset;
}

uint16_t RenderRasterDEMSource::getTileSize() const {
    return resolveTileSize(impl().getTileSize());
}

void RenderRasterDEMSource::updateInternal(const Tileset& tileset,
                                           const std::vector<Immutable<LayerProperties>>& layers,
                                           const bool needsRendering,
                                           const bool needsRelayout,
                                           const TileParameters& parameters) {
    // RenderTileSetSource::update drops every tile when the tileset changes, so the ranges
    // decoded from them go too.
    if (elevationRangesTileset != tileset) {
        elevationRanges.clear();
        overallElevationRange.reset();
        elevationRangesTileset = tileset;
    }

    tilePyramid.update(layers,
                       needsRendering,
                       needsRelayout,
                       parameters,
                       *baseImpl,
                       resolveTileSize(impl().getTileSize()),
                       tileset.zoomRange,
                       tileset.bounds,
                       [&](const OverscaledTileID& tileID, TileObserver* observer_) {
                           return std::make_unique<RasterDEMTile>(tileID, baseImpl->id, parameters, tileset, observer_);
                       });
    algorithm::updateTileMasks(tilePyramid.getRenderedTiles());
}

std::optional<Range<int32_t>> RenderRasterDEMSource::getElevationRange(const CanonicalTileID& id) const {
    if (elevationRanges.empty()) {
        return std::nullopt;
    }
    // Ranges are only ever kept for tiles of the tileset's zoom range.
    const auto& zoomRange = elevationRangesTileset->zoomRange;
    for (int z = std::min(id.z, zoomRange.max); z >= zoomRange.min; --z) {
        const int dz = id.z - z;
        const auto it = elevationRanges.find(CanonicalTileID(static_cast<uint8_t>(z), id.x >> dz, id.y >> dz));
        if (it != elevationRanges.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}

void RenderRasterDEMSource::addElevationRange(const CanonicalTileID& id, const Range<int32_t> range) {
    elevationRanges.insert_or_assign(id, range);
    overallElevationRange = overallElevationRange ? Range<int32_t>{std::min(overallElevationRange->min, range.min),
                                                                   std::max(overallElevationRange->max, range.max)}
                                                  : range;
}

void RenderRasterDEMSource::keepElevationRangesOfHeldTiles() {
    elevationRanges.clear();
    overallElevationRange.reset();
    const auto addHeld = [&](const std::map<OverscaledTileID, std::unique_ptr<Tile>>& tiles) {
        for (const auto& [id, tile] : tiles) {
            if (const auto* bucket = static_cast<const RasterDEMTile&>(*tile).getBucket()) {
                addElevationRange(id.canonical, elevationRangeOf(*bucket));
            }
        }
    };
    addHeld(tilePyramid.getTiles());
    addHeld(tilePyramid.getCache().getTiles());
}

void RenderRasterDEMSource::onTileChanged(Tile& tile) {
    auto& demtile = static_cast<RasterDEMTile&>(tile);

    std::map<DEMTileNeighbors, DEMTileNeighbors> opposites = {
        {DEMTileNeighbors::Left, DEMTileNeighbors::Right},
        {DEMTileNeighbors::Right, DEMTileNeighbors::Left},
        {DEMTileNeighbors::TopLeft, DEMTileNeighbors::BottomRight},
        {DEMTileNeighbors::TopCenter, DEMTileNeighbors::BottomCenter},
        {DEMTileNeighbors::TopRight, DEMTileNeighbors::BottomLeft},
        {DEMTileNeighbors::BottomRight, DEMTileNeighbors::TopLeft},
        {DEMTileNeighbors::BottomCenter, DEMTileNeighbors::TopCenter},
        {DEMTileNeighbors::BottomLeft, DEMTileNeighbors::TopRight}};

    if (tile.isRenderable() && demtile.neighboringTiles != DEMTileNeighbors::Complete) {
        const CanonicalTileID canonical = tile.id.canonical;
        const auto dim = static_cast<uint32_t>(std::pow(2, canonical.z));
        const uint32_t px = (canonical.x - 1 + dim) % dim;
        const int pxw = canonical.x == 0 ? tile.id.wrap - 1 : tile.id.wrap;
        const uint32_t nx = (canonical.x + 1 + dim) % dim;
        const int nxw = (canonical.x + 1 == dim) ? tile.id.wrap + 1 : tile.id.wrap;

        auto getNeighbor = [&](DEMTileNeighbors mask) {
            if (mask == DEMTileNeighbors::Left) {
                return OverscaledTileID(tile.id.overscaledZ, pxw, canonical.z, px, canonical.y);
            } else if (mask == DEMTileNeighbors::Right) {
                return OverscaledTileID(tile.id.overscaledZ, nxw, canonical.z, nx, canonical.y);
            } else if (mask == DEMTileNeighbors::TopLeft) {
                return OverscaledTileID(tile.id.overscaledZ, pxw, canonical.z, px, canonical.y - 1);
            } else if (mask == DEMTileNeighbors::TopCenter) {
                return OverscaledTileID(tile.id.overscaledZ, tile.id.wrap, canonical.z, canonical.x, canonical.y - 1);
            } else if (mask == DEMTileNeighbors::TopRight) {
                return OverscaledTileID(tile.id.overscaledZ, nxw, canonical.z, nx, canonical.y - 1);
            } else if (mask == DEMTileNeighbors::BottomLeft) {
                return OverscaledTileID(tile.id.overscaledZ, pxw, canonical.z, px, canonical.y + 1);
            } else if (mask == DEMTileNeighbors::BottomCenter) {
                return OverscaledTileID(tile.id.overscaledZ, tile.id.wrap, canonical.z, canonical.x, canonical.y + 1);
            } else if (mask == DEMTileNeighbors::BottomRight) {
                return OverscaledTileID(tile.id.overscaledZ, nxw, canonical.z, nx, canonical.y + 1);
            } else {
                throw std::runtime_error("mask is not a valid tile neighbor");
            }
        };

        for (uint8_t i = 0; i < 8; i++) {
            auto mask = DEMTileNeighbors(std::pow(2, i));
            // only backfill if this neighbor has not been previously backfilled
            if ((demtile.neighboringTiles & mask) != mask) {
                OverscaledTileID neighborid = getNeighbor(mask);
                Tile* renderableNeighbor = tilePyramid.getTile(neighborid);
                if (renderableNeighbor != nullptr && renderableNeighbor->isRenderable()) {
                    auto& borderTile = static_cast<RasterDEMTile&>(*renderableNeighbor);
                    demtile.backfillBorder(borderTile, mask);

                    // if the border tile has not been backfilled by a previous
                    // instance of the main tile, backfill its corresponding
                    // neighbor as well.
                    const DEMTileNeighbors& borderMask = opposites[mask];
                    if ((borderTile.neighboringTiles & borderMask) != borderMask) {
                        borderTile.backfillBorder(demtile, borderMask);
                    }
                }
            }
        }
    }

    // Keep the tile's elevation range for the tile covers (see getElevationRange).
    if (const auto* bucket = demtile.getBucket()) {
        if (elevationRanges.size() >= maxElevationRanges) {
            keepElevationRangesOfHeldTiles();
        }
        addElevationRange(tile.id.canonical, elevationRangeOf(*bucket));
    }
    RenderTileSource::onTileChanged(tile);
}

std::unordered_map<std::string, std::vector<Feature>> RenderRasterDEMSource::queryRenderedFeatures(
    const ScreenLineString&,
    const TransformState&,
    const std::unordered_map<std::string, const RenderLayer*>&,
    const RenderedQueryOptions&,
    const mat4&) const {
    return std::unordered_map<std::string, std::vector<Feature>>{};
}

std::vector<Feature> RenderRasterDEMSource::querySourceFeatures(const SourceQueryOptions&) const {
    return {};
}

} // namespace mln
