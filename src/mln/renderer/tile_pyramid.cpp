#include <mln/renderer/tile_pyramid.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_source.hpp>
#include <mln/renderer/tile_parameters.hpp>
#include <mln/renderer/query.hpp>
#include <mln/map/transform.hpp>
#include <mln/map/vertical_perspective_projection.hpp>
#include <mln/math/clamp.hpp>
#include <mln/math/log2.hpp>
#include <mln/actor/scheduler.hpp>
#include <mln/util/tile_cover.hpp>
#include <mln/util/tile_range.hpp>
#include <mln/util/enum.hpp>
#include <mln/util/logging.hpp>

#include <mln/algorithm/update_renderables.hpp>

#include <mapbox/geometry/envelope.hpp>

#include <cmath>
#include <algorithm>
#include <array>
#include <optional>
#include <span>
#include <unordered_map>

namespace mln {

using namespace style;

namespace {
TileObserver nullObserver;
const std::map<OverscaledTileID, std::unique_ptr<Tile>> emptyPrefetchedTiles;
} // namespace

TilePyramid::TilePyramid(const TaggedScheduler& threadPool_)
    : cache(threadPool_),
      observer(&nullObserver) {}

TilePyramid::~TilePyramid() = default;

bool TilePyramid::isLoaded() const {
    for (const auto& pair : tiles) {
        if (!pair.second->isComplete()) {
            return false;
        }
    }

    return true;
}

Tile* TilePyramid::getTile(const OverscaledTileID& tileID) {
    auto it = tiles.find(tileID);
    return it == tiles.end() ? cache.get(tileID) : it->second.get();
}

const Tile* TilePyramid::getRenderedTile(const UnwrappedTileID& tileID) const {
    auto it = renderedTiles.find(tileID);
    return it != renderedTiles.end() ? &it->second.get() : nullptr;
}

void TilePyramid::update(const std::vector<Immutable<style::LayerProperties>>& layers,
                         const bool needsRendering,
                         const bool needsRelayout,
                         const TileParameters& parameters,
                         const style::Source::Impl& sourceImpl,
                         const uint16_t tileSize,
                         const Range<uint8_t> zoomRange,
                         std::optional<LatLngBounds> bounds,
                         std::function<std::unique_ptr<Tile>(const OverscaledTileID&, TileObserver*)> createTile) {
    const auto& subdivisionGranularity = parameters.subdivisionGranularity;
    const bool relayout = needsRelayout || subdivisionGranularity != lastSubdivisionGranularity;
    lastSubdivisionGranularity = subdivisionGranularity;

    // If we need a relayout, abandon any cached tiles; they're now stale.
    if (relayout) {
        cache.clear();
    }

    // If we're not going to render anything, move our existing tiles into
    // the cache (if they're not stale) or abandon them, and return.
    if (!needsRendering) {
        for (auto& entry : tiles) {
            if (!relayout) {
                // These tiles are invisible, we set optional necessity
                // for them and thus suppress network requests on
                // tiles expiration (see `OnlineFileRequest`).
                entry.second->setNecessity(TileNecessity::Optional);
                cache.add(entry.first, std::move(entry.second));
            } else {
                cache.deferredRelease(std::move(entry.second));
            }
        }

        tiles.clear();
        renderedTiles.clear();
        cache.deferPendingReleases();

        return;
    }

    handleWrapJump(static_cast<float>(parameters.transformState.getLatLng().longitude()));

    // Optionally shift the zoom level
    double zoom = util::clamp<double>(parameters.transformState.getZoom() + parameters.tileLodZoomShift,
                                      parameters.transformState.getMinZoom(),
                                      parameters.transformState.getMaxZoom());

    const auto type = sourceImpl.type;
    // Determine the overzooming/underzooming amounts and required tiles.
    int32_t overscaledZoom = util::coveringZoomLevel(zoom, type, tileSize);
    int32_t tileZoom = overscaledZoom;
    int32_t panZoom = zoomRange.max;

    const std::optional<uint8_t>& sourcePrefetchZoomDelta = sourceImpl.getPrefetchZoomDelta();
    const std::optional<uint8_t>& maxParentTileOverscaleFactor = sourceImpl.getMaxOverscaleFactorForParentTiles();
    const Duration minimumUpdateInterval = sourceImpl.getMinimumTileUpdateInterval();
    const bool isVolatile = sourceImpl.isVolatile();

    std::vector<OverscaledTileID> idealTiles;
    std::vector<OverscaledTileID> panTiles;

    util::TileCoverParameters tileCoverParameters = {
        .transformState = parameters.transformState,
        .tileLodMinRadius = parameters.tileLodMinRadius,
        .tileLodScale = parameters.tileLodScale,
        .tileLodPitchThreshold = parameters.tileLodPitchThreshold,
        .tileLodMode = parameters.tileLodMode,
        .requestedZoom = zoom + util::log2(util::tileSize_D / tileSize),
        .roundZoom = type == SourceType::Raster || type == SourceType::RasterDEM || type == SourceType::Video};

    if (std::cmp_greater_equal(overscaledZoom, zoomRange.min)) {
        int32_t idealZoom = std::min<int32_t>(zoomRange.max, overscaledZoom);

        // Make sure we're not reparsing overzoomed raster tiles.
        if (type == SourceType::Raster) {
            tileZoom = idealZoom;
        }

        // Only attempt prefetching in continuous mode.
        if (parameters.mode == MapMode::Continuous && type != style::SourceType::GeoJSON &&
            type != style::SourceType::Annotations) {
            // Request lower zoom level tiles (if configured to do so) in an attempt
            // to show something on the screen faster at the cost of a little of bandwidth.
            const uint8_t prefetchZoomDelta = sourcePrefetchZoomDelta ? *sourcePrefetchZoomDelta
                                                                      : parameters.prefetchZoomDelta;
            if (prefetchZoomDelta) {
                panZoom = std::max<int32_t>(tileZoom - prefetchZoomDelta, zoomRange.min);
            }

            if (panZoom < idealZoom) {
                // The prefetch's level of detail falls off from its own, coarser zoom.
                util::TileCoverParameters panParameters = tileCoverParameters;
                panParameters.requestedZoom.reset();
                panParameters.roundZoom = false;
                panTiles = util::tileCover(panParameters, panZoom, zoomRange);
            }
        }

        idealTiles = util::tileCover(tileCoverParameters, idealZoom, zoomRange, tileZoom);
        if (parameters.mode == MapMode::Tile && type != SourceType::Raster && type != SourceType::RasterDEM &&
            idealTiles.size() > 1) {
            mln::Log::Warning(mln::Event::General,
                              "Provided camera options returned " + std::to_string(idealTiles.size()) +
                                  " tiles, only " + util::toString(idealTiles[0]) + " is taken in Tile mode.");
            idealTiles = {idealTiles[0]};
        }
    }

    // Stores a list of all the tiles that we're definitely going to retain.
    // There are two kinds of tiles we need: the ideal tiles determined by the
    // tile cover. They may not yet be in use because they're still loading. In
    // addition to that, we also need to retain all tiles that we're actively
    // using, e.g. as a replacement for tile that aren't loaded yet.
    std::set<OverscaledTileID> retain;

    auto retainTileFn = [&](Tile& tile, TileNecessity necessity) -> void {
        if (retain.emplace(tile.id).second) {
            tile.setUpdateParameters({.minimumUpdateInterval = minimumUpdateInterval, .isVolatile = isVolatile});
            tile.setNecessity(necessity);
        }

        if (relayout) {
            tile.setSubdivisionGranularity(subdivisionGranularity);
            tile.setLayers(layers, parameters.globalState);
        }
    };
    auto getTileFn = [&](const OverscaledTileID& tileID) -> Tile* {
        auto it = tiles.find(tileID);
        return it == tiles.end() ? nullptr : it->second.get();
    };

    // The min and max zoom for TileRange are based on the updateRenderables
    // algorithm. Tiles are created at the ideal tile zoom or at lower zoom
    // levels. Child tiles are used from the cache, but not created.
    // A cover that picks each tile's zoom by its distance from the camera asks for tiles finer than the nominal
    // zoom, up to the source's maximum; the globe cover does, like `TileLodMode::Distance`.
    std::optional<util::TileRange> tileRange = std::nullopt;
    if (bounds) {
        const bool variableZoom = parameters.tileLodMode == TileLodMode::Distance ||
                                  parameters.transformState.isGlobeRendering();
        const int32_t maxZoom = variableZoom ? zoomRange.max : std::min(tileZoom, static_cast<int32_t>(zoomRange.max));
        tileRange = util::TileRange::fromLatLngBounds(*bounds, zoomRange.min, maxZoom);
    }
    auto createTileFn = [&](const OverscaledTileID& tileID) -> Tile* {
        if (tileRange && !tileRange->contains(tileID.canonical)) {
            return nullptr;
        }
        std::unique_ptr<Tile> tile = cache.pop(tileID);
        if (!tile) {
            tile = createTile(tileID, observer);
            if (!tile) return nullptr;
            tile->setSubdivisionGranularity(subdivisionGranularity);
            tile->setLayers(layers, parameters.globalState);
        }

        return tiles.emplace(tileID, std::move(tile)).first->second.get();
    };

    auto previouslyRenderedTiles = std::move(renderedTiles);

    auto renderTileFn = [&](const UnwrappedTileID& tileID, Tile& tile) {
        addRenderTile(tileID, tile);
        previouslyRenderedTiles.erase(tileID); // Still rendering this tile, no need for special fading logic.
        tile.markRenderedIdeal();
    };

    renderedTiles.clear();

    if (!panTiles.empty()) {
        algorithm::updateRenderables(
            getTileFn,
            createTileFn,
            retainTileFn,
            [](const UnwrappedTileID&, Tile&) {},
            panTiles,
            emptyPrefetchedTiles,
            zoomRange,
            maxParentTileOverscaleFactor);
    }

    algorithm::updateRenderables(getTileFn,
                                 createTileFn,
                                 retainTileFn,
                                 renderTileFn,
                                 idealTiles,
                                 tiles,
                                 zoomRange,
                                 maxParentTileOverscaleFactor);

    for (auto previouslyRenderedTile : previouslyRenderedTiles) {
        Tile& tile = previouslyRenderedTile.second;
        tile.markRenderedPreviously();
        if (tile.holdForFade()) {
            // Since it was rendered in the last frame, we know we have it
            // Don't mark the tile "Required" to avoid triggering a new network request
            retainTileFn(tile, TileNecessity::Optional);
            addRenderTile(previouslyRenderedTile.first, tile);
        }
    }

    if (type != SourceType::Annotations && cacheEnabled) {
        auto conservativeCacheSize = static_cast<size_t>(
            std::max(static_cast<double>(parameters.transformState.getSize().width) / tileSize, 1.0) *
            std::max(static_cast<double>(parameters.transformState.getSize().height) / tileSize, 1.0) *
            (parameters.transformState.getMaxZoom() - parameters.transformState.getMinZoom() + 1) * 0.5);
        cache.setSize(conservativeCacheSize);
    } else {
        cache.setSize(0);
    }

    // Remove stale tiles. This goes through the (sorted!) tiles map and retain
    // set in lockstep and removes items from tiles that don't have the
    // corresponding key in the retain set.
    {
        auto tilesIt = tiles.begin();
        auto retainIt = retain.begin();
        while (tilesIt != tiles.end()) {
            if (retainIt == retain.end() || tilesIt->first < *retainIt) {
                // Remove the tile from the map.
                // If it requires re-layout, discard it asynchronously, otherwise keep it in the cache
                const auto key = tilesIt->first;
                if (std::unique_ptr<Tile> tile = std::move(tiles.extract(tilesIt++).mapped())) {
                    if (relayout) {
                        cache.deferredRelease(std::move(tile));
                    } else {
                        tile->setNecessity(TileNecessity::Optional);
                        cache.add(key, std::move(tile));
                    }
                }
            } else {
                if (!(*retainIt < tilesIt->first)) {
                    ++tilesIt;
                }
                ++retainIt;
            }
        }
    }

    for (auto& pair : tiles) {
        pair.second->setShowCollisionBoxes(parameters.debugOptions & MapDebugOptions::Collision);
    }

    // Initialize renderable tiles and update the contained layer render data.
    for (auto& entry : renderedTiles) {
        Tile& tile = entry.second;
        assert(tile.isRenderable());
        tile.usedByRenderedLayers = false;

        const bool holdForFade = tile.holdForFade();
        for (const auto& layerProperties : layers) {
            const auto* typeInfo = layerProperties->baseImpl->getTypeInfo();
            if (holdForFade && typeInfo->fadingTiles == LayerTypeInfo::FadingTiles::NotRequired) {
                continue;
            }
            tile.usedByRenderedLayers |= tile.layerPropertiesUpdated(layerProperties);
        }
    }

    cache.deferPendingReleases();
}

void TilePyramid::handleWrapJump(float lng) {
    // On top of the regular z/x/y values, TileIDs have a `wrap` value that specify
    // which cppy of the world the tile belongs to. For example, at `lng: 10` you
    // might render z/x/y/0 while at `lng: 370` you would render z/x/y/1.
    //
    // When lng values get wrapped (going from `lng: 370` to `long: 10`) you expect
    // to see the same thing on the screen (370 degrees and 10 degrees is the same
    // place in the world) but all the TileIDs will have different wrap values.
    //
    // In order to make this transition seamless, we calculate the rounded difference of
    // "worlds" between the last frame and the current frame. If the map panned by
    // a world, then we can assign all the tiles new TileIDs with updated wrap values.
    // For example, assign z/x/y/1 a new id: z/x/y/0. It is the same tile, just rendered
    // in a different position.
    //
    // This enables us to reuse the tiles at more ideal locations and prevent flickering.

    const float lngDifference = lng - prevLng;
    const float worldDifference = lngDifference / 360.f;
    const auto wrapDelta = static_cast<int16_t>(std::round(worldDifference));
    prevLng = lng;

    if (wrapDelta) {
        std::map<OverscaledTileID, std::unique_ptr<Tile>> newTiles;
        std::map<UnwrappedTileID, std::reference_wrapper<Tile>> newRenderTiles;
        for (auto& tile : tiles) {
            auto newID = tile.second->id.unwrapTo(tile.second->id.wrap + wrapDelta);
            tile.second->id = newID;
            newTiles.emplace(newID, std::move(tile.second));
        }
        tiles = std::move(newTiles);

        for (auto& tile : renderedTiles) {
            UnwrappedTileID newID = tile.first.unwrapTo(tile.first.wrap + wrapDelta);
            newRenderTiles.emplace(newID, tile.second);
        }
        renderedTiles = std::move(newRenderTiles);
    }
}

std::unordered_map<std::string, std::vector<Feature>> TilePyramid::queryRenderedFeatures(
    const ScreenLineString& geometry,
    const TransformState& transformState,
    const std::unordered_map<std::string, const RenderLayer*>& layers,
    const RenderedQueryOptions& options,
    const GlobalStateMap* globalState,
    const mat4& projMatrix,
    const SourceFeatureState& featureState) const {
    std::unordered_map<std::string, std::vector<Feature>> result;
    if (renderedTiles.empty() || geometry.empty()) {
        return result;
    }

    const auto toWorld = [&](const ScreenCoordinate& p) {
        return TileCoordinate::fromScreenCoordinate(transformState, 0, {p.x, transformState.getSize().height - p.y}).p;
    };
    const bool globe = transformState.isGlobeRendering();
    const auto onPlanet = [&](const ScreenCoordinate& p) {
        return VerticalPerspectiveProjection::screenCoordinateHitsGlobe(transformState,
                                                                        {p.x, transformState.getSize().height - p.y});
    };

    // Past the planet's edge a query meets the planet only at the horizon, and a point there stands for the nearest
    // horizon point, so a query's corners alone would cut the planet's limb off; its edges are followed in steps.
    ScreenLineString traced;
    if (globe && geometry.size() > 1 && !std::ranges::all_of(geometry, onPlanet)) {
        constexpr int steps = 32;
        for (std::size_t i = 0; i + 1 < geometry.size(); ++i) {
            for (int k = 0; k < steps; ++k) {
                const double t = static_cast<double>(k) / steps;
                traced.push_back({geometry[i].x + (geometry[i + 1].x - geometry[i].x) * t,
                                  geometry[i].y + (geometry[i + 1].y - geometry[i].y) * t});
            }
        }
        traced.push_back(geometry.back());
    }
    const ScreenLineString& screenGeometry = traced.empty() ? geometry : traced;
    LineString<double> queryGeometry;
    queryGeometry.reserve(screenGeometry.size());

    for (const auto& p : screenGeometry) {
        queryGeometry.push_back(toWorld(p));
    }

    if (globe) {
        // GL JS's `transformBbox`: the globe has no world copies, so a box across the antimeridian lands on the rest
        // of the world, which shows when its slightly shrunken corners fall outside it. It moves to the copy west of
        // the main world.
        auto screenBox = mapbox::geometry::envelope(geometry);
        const double shrink = std::min(screenBox.max.x - screenBox.min.x, screenBox.max.y - screenBox.min.y) * 0.001;
        screenBox.min.x += shrink;
        screenBox.min.y += shrink;
        screenBox.max.x -= shrink;
        screenBox.max.y -= shrink;
        const auto bounds = mapbox::geometry::envelope(queryGeometry);
        // A corner past the planet stands for the nearest point on the horizon, which tells nothing about the
        // antimeridian, so GL JS's check lost boxes that reach into space, such as the whole view of a small globe.
        // Such corners are left out, and the box's center, when it is on the planet, decides by longitude for them.
        bool covered = true;
        bool reachesIntoSpace = false;
        for (const ScreenCoordinate& corner : std::array<ScreenCoordinate, 4>{screenBox.min,
                                                                              {screenBox.max.x, screenBox.min.y},
                                                                              {screenBox.min.x, screenBox.max.y},
                                                                              screenBox.max}) {
            if (!onPlanet(corner)) {
                reachesIntoSpace = true;
                continue;
            }
            const auto c = toWorld(corner);
            covered = covered && bounds.min.x <= c.x && c.x <= bounds.max.x && bounds.min.y <= c.y &&
                      c.y <= bounds.max.y;
        }
        const ScreenCoordinate center{(screenBox.min.x + screenBox.max.x) / 2, (screenBox.min.y + screenBox.max.y) / 2};
        if (reachesIntoSpace && onPlanet(center)) {
            const auto c = toWorld(center);
            covered = covered && bounds.min.x <= c.x && c.x <= bounds.max.x;
        }
        if (!covered) {
            for (auto& c : queryGeometry) {
                if (c.x > 0.5) {
                    c.x -= 1.0;
                }
            }
        }
    }

    mapbox::geometry::box<double> box = mapbox::geometry::envelope(queryGeometry);

    // GL JS sorts the globe's query tiles by their copy on the main world, so there the wrap only breaks ties.
    auto cmp = [globe](const UnwrappedTileID& a, const UnwrappedTileID& b) {
        if (globe) {
            return std::tie(a.canonical.z, a.canonical.y, a.canonical.x, a.wrap) <
                   std::tie(b.canonical.z, b.canonical.y, b.canonical.x, b.wrap);
        }
        return std::tie(a.canonical.z, a.canonical.y, a.wrap, a.canonical.x) <
               std::tie(b.canonical.z, b.canonical.y, b.wrap, b.canonical.x);
    };

    std::map<UnwrappedTileID, std::reference_wrapper<Tile>, decltype(cmp)> sortedTiles{
        renderedTiles.begin(), renderedTiles.end(), cmp};

    auto maxPitchScaleFactor = transformState.maxPitchScaleFactor();

    // The globe returns a feature once per tile, as GL JS does, though the tile is tested at three copies and can be
    // rendered at two wraps: these are where this tile's features start in each layer's results.
    std::optional<CanonicalTileID> canonicalTile;
    std::unordered_map<std::string, std::size_t> canonicalTileStart;

    for (const auto& entry : sortedTiles) {
        const UnwrappedTileID& id = entry.first;
        Tile& tile = entry.second;

        const auto scale = static_cast<float>(transformState.getScale() /
                                              (1 << id.canonical.z)); // equivalent to std::pow(2,
                                                                      // transformState.getZoom() - id.canonical.z);
        auto queryPadding = maxPitchScaleFactor * tile.getQueryPadding(layers) * util::EXTENT / util::tileSize_D /
                            scale;

        if (globe && canonicalTile != id.canonical) {
            canonicalTile = id.canonical;
            canonicalTileStart.clear();
            for (const auto& [layerID, features] : result) {
                canonicalTileStart.emplace(layerID, features.size());
            }
        }

        // The query geometry lies on the main world or, across the antimeridian, west of it, and its padding reaches
        // past either side of it: a globe tile is tested at its copies west of, on and east of the main world. GL JS
        // leaves out the east copy and misses features just east of the antimeridian under a query just west of it.
        const std::array<UnwrappedTileID, 3> globeCopies{
            UnwrappedTileID(-1, id.canonical), UnwrappedTileID(0, id.canonical), UnwrappedTileID(1, id.canonical)};
        for (const auto& copy :
             globe ? std::span<const UnwrappedTileID>(globeCopies) : std::span<const UnwrappedTileID>(&id, 1)) {
            GeometryCoordinate tileSpaceBoundsMin = TileCoordinate::toGeometryCoordinate(copy, box.min);
            if (tileSpaceBoundsMin.x - queryPadding >= util::EXTENT ||
                tileSpaceBoundsMin.y - queryPadding >= util::EXTENT) {
                continue;
            }

            GeometryCoordinate tileSpaceBoundsMax = TileCoordinate::toGeometryCoordinate(copy, box.max);
            if (tileSpaceBoundsMax.x + queryPadding < 0 || tileSpaceBoundsMax.y + queryPadding < 0) {
                continue;
            }

            GeometryCoordinates tileSpaceQueryGeometry;
            tileSpaceQueryGeometry.reserve(queryGeometry.size());
            for (const auto& c : queryGeometry) {
                tileSpaceQueryGeometry.push_back(TileCoordinate::toGeometryCoordinate(copy, c));
            }

            if (!globe) {
                tile.queryRenderedFeatures(result,
                                           tileSpaceQueryGeometry,
                                           transformState,
                                           layers,
                                           options,
                                           globalState,
                                           projMatrix,
                                           featureState);
                continue;
            }

            std::unordered_map<std::string, std::vector<Feature>> found;
            tile.queryRenderedFeatures(
                found, tileSpaceQueryGeometry, transformState, layers, options, globalState, projMatrix, featureState);
            for (auto& [layerID, features] : found) {
                auto& layerResult = result[layerID];
                const auto start = canonicalTileStart.contains(layerID) ? canonicalTileStart.at(layerID) : 0;
                const auto end = layerResult.size();
                for (auto& feature : features) {
                    if (std::none_of(layerResult.begin() + start, layerResult.begin() + end, [&](const Feature& other) {
                            return other == feature;
                        })) {
                        layerResult.push_back(std::move(feature));
                    }
                }
            }
        }
    }

    return result;
}

std::vector<Feature> TilePyramid::querySourceFeatures(const SourceQueryOptions& options,
                                                      const GlobalStateMap* globalState) const {
    std::vector<Feature> result;

    for (const auto& pair : tiles) {
        pair.second->querySourceFeatures(result, options, globalState);
    }

    return result;
}

void TilePyramid::setCacheEnabled(bool enable) {
    cacheEnabled = enable;
}

void TilePyramid::reduceMemoryUse() {
    cache.clear();
}

void TilePyramid::setObserver(TileObserver* observer_) {
    observer = observer_;
}

void TilePyramid::dumpDebugLogs() const {
    for (const auto& pair : tiles) {
        pair.second->dumpDebugLogs();
    }
}

void TilePyramid::clearAll() {
    fadingTiles = false;
    tiles.clear();
    renderedTiles.clear();
    cache.clear();
}

void TilePyramid::addRenderTile(const UnwrappedTileID& tileID, Tile& tile) {
    assert(tile.isRenderable());
    renderedTiles.emplace(tileID, tile);
}

void TilePyramid::updateFadingTiles() {
    fadingTiles = false;
    for (auto& entry : renderedTiles) {
        Tile& tile = entry.second;
        if (tile.holdForFade()) {
            fadingTiles = true;
            tile.performedFadePlacement();
        }
    }
}

} // namespace mln
