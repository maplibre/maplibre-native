#include <mbgl/renderer/sources/render_contour_source.hpp>
#include <mbgl/algorithm/contour/intervals.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/tile/contour_tile.hpp>
#include <mbgl/tile/raster_dem_tile.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/logging.hpp>

#include <cmath>

namespace mbgl {

using namespace style;

namespace {

// Resolve the source's `majorMultiplier` schedule at a tile's canonical
// zoom to a single positive integer. Returns 0 when the schedule is
// empty or invalid (callers treat 0 as "never major").
std::int64_t resolveMajorMultiplier(const algorithm::contour::IntervalSchedule& schedule, double zoom) {
    const double v = algorithm::contour::intervalForZoom(schedule, zoom);
    if (v <= 0.0) return 0;
    return static_cast<std::int64_t>(std::llround(v));
}

} // namespace

RenderContourSource::RenderContourSource(Immutable<style::ContourSource::Impl> impl_, const TaggedScheduler& threadPool_)
    : RenderTileSource(std::move(impl_), threadPool_) {}

RenderContourSource::~RenderContourSource() {
    // Intentionally do NOT call upstream->removeTileLoadListener here.
    // Sibling-source destruction order in RenderOrchestrator's
    // std::unordered_map<std::string, std::unique_ptr<RenderSource>> is
    // unspecified, so by the time we run, the upstream raster-dem source
    // may already have been destroyed — dereferencing `upstream` would be
    // a use-after-free. The listener_set is owned by upstream, so any
    // still-registered listener is destroyed when upstream itself is.
    // The listener closure captures only a WeakPtr to this source, so it
    // safely no-ops if it fires (e.g. mid-notify) after we're gone.
}

const style::ContourSource::Impl& RenderContourSource::impl() const {
    return static_cast<const style::ContourSource::Impl&>(*baseImpl);
}

void RenderContourSource::update(Immutable<style::Source::Impl> baseImpl_,
                                 const std::vector<Immutable<LayerProperties>>& layers,
                                 const bool needsRendering,
                                 const bool needsRelayout,
                                 const TileParameters& parameters) {
    std::swap(baseImpl, baseImpl_);
    enabled = needsRendering;

    rebindUpstream(parameters);

    // Mirror the upstream DEM source's zoom range so contour tiles share
    // the DEM's canonical zoom (e.g. canonical.z = DEM maxzoom = 12 for a
    // Terrarium source). At display zoom > maxzoom the renderer overscales
    // both pyramids in lock-step — same canonical tile, same line geometry
    // reused for the bigger screen area. Diverging the two pyramids' zoom
    // ranges leaves the contour pyramid asking for tiles at coords that
    // have no upstream DEM, with no matching listener fire — empty tiles.
    //
    // Default to (0..12) when the upstream's zoom range isn't available
    // yet (style still loading) so the very first frame doesn't go
    // through a wide-range path that has nothing to populate from.
    Range<std::uint8_t> contourZoomRange{0, 12};
    if (upstream != nullptr) {
        if (auto upstreamRange = upstream->getZoomRange()) {
            contourZoomRange = *upstreamRange;
        }
    }
    tilePyramid.update(layers,
                       needsRendering,
                       needsRelayout,
                       parameters,
                       *baseImpl,
                       util::tileSize_I,
                       contourZoomRange,
                       std::optional<LatLngBounds>{},
                       [&](const OverscaledTileID& tileID, TileObserver* observer_) {
                           auto tile = std::make_unique<ContourTile>(tileID, impl().id, parameters, observer_);
                           // If the upstream DEM has already loaded this coord, populate
                           // synchronously — closes the race between contour-pyramid update
                           // and DEM tile arrivals (the listener already fired earlier and
                           // had no pyramid tile to target).
                           if (upstream != nullptr) {
                               if (const RasterDEMTile* dem = upstream->getRenderableTile(tileID)) {
                                   const auto& opts = impl().getOptions();
                                   const double zoom = static_cast<double>(tileID.canonical.z);
                                   const double interval = algorithm::contour::intervalForZoom(opts.intervals, zoom);
                                   const std::int64_t major = resolveMajorMultiplier(opts.majorMultiplier, zoom);
                                   tile->populateFromDEM(*dem, interval, major, opts.unit);
                               }
                           }
                           return tile;
                       });
}

void RenderContourSource::rebindUpstream(const TileParameters& parameters) {
    // Resolve the upstream raster-dem source by ID for this frame. A null
    // result means the upstream source has been removed from the style or
    // hasn't been added yet — either way, our cached pointer must drop.
    RenderRasterDEMSource* resolved = nullptr;
    if (parameters.getRenderSource) {
        if (RenderSource* found = parameters.getRenderSource(impl().getOptions().sourceID)) {
            if (found->baseImpl->type == SourceType::RasterDEM) {
                resolved = static_cast<RenderRasterDEMSource*>(found);
            } else {
                Log::Warning(Event::Style,
                             "contour source `" + impl().id + "` references `" + impl().getOptions().sourceID +
                                 "` which is not a raster-dem source");
            }
        }
    }

    if (resolved == upstream) {
        return; // No change — keep existing subscription as-is.
    }

    // Identity changed (added, removed, or swapped). Drop the old handle
    // without dereferencing the (possibly destroyed) previous upstream —
    // its listener_set was destroyed with it, so any registered listener
    // is already gone. Only the handle integer remains, and we can safely
    // forget it.
    listenerHandle.reset();
    upstream = resolved;
    if (upstream != nullptr) {
        // Capture both `this` (for member access) and a WeakPtr (for
        // liveness). The lock-and-check pattern keeps `this` pinned during
        // the call and bails out if the source has been destroyed — the
        // listener_set may outlive us briefly during multi-source teardown.
        listenerHandle = upstream->addTileLoadListener(
            [this, weak = weakFactory.makeWeakPtr()](const RasterDEMTile& demTile) {
                if (auto guard = weak.lock(); weak) {
                    onUpstreamTileLoaded(demTile);
                }
            });
    }
}

void RenderContourSource::onUpstreamTileLoaded(const RasterDEMTile& demTile) {
    Tile* match = tilePyramid.getTile(demTile.id);
    if (match == nullptr) {
        // No contour tile at this coord yet — the createTile callback will
        // catch it on the next pyramid update via getRenderableTile, so
        // missing the listener fire is harmless.
        return;
    }
    const auto& opts = impl().getOptions();
    const double zoom = static_cast<double>(demTile.id.canonical.z);
    const double interval = algorithm::contour::intervalForZoom(opts.intervals, zoom);
    const std::int64_t major = resolveMajorMultiplier(opts.majorMultiplier, zoom);
    static_cast<ContourTile&>(*match).populateFromDEM(demTile, interval, major, opts.unit);
}

} // namespace mbgl
