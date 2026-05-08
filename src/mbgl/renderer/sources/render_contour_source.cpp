#include <mbgl/renderer/sources/render_contour_source.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/tile/contour_tile.hpp>
#include <mbgl/tile/raster_dem_tile.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {

using namespace style;

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

    // For the spike, drive contour-tile coords from the *upstream DEM
    // source's* visible tile coords rather than computing our own viewport
    // tile range. This matches the user's stated requirement: "DEM tiles get
    // requested at the correct zoom level (driven by their resolution); we
    // can always display vector tiles at a higher / lower zoom level." The
    // contour pyramid grows lock-step with the DEM pyramid.
    //
    // We use a wide zoom range and let the DEM-tile-load listener call
    // populateFromDEM on the corresponding pyramid entry. Tiles created here
    // before their DEM counterpart loads remain in pending state (no
    // bucket); they become renderable only when the listener fires.
    constexpr Range<std::uint8_t> contourZoomRange{0, 22};
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
                                   tile->populateFromDEM(*dem);
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
        // No contour tile at this coord yet — the pyramid doesn't have a
        // matching visible tile. The next pyramid update will create one,
        // and we'll need a re-fire of populateFromDEM at that point. For the
        // spike that's tolerable; Task 4 will add a small "pending coords"
        // cache so newly-created tiles get populated synchronously from any
        // already-loaded DEM data.
        return;
    }
    static_cast<ContourTile&>(*match).populateFromDEM(demTile);
}

} // namespace mbgl
