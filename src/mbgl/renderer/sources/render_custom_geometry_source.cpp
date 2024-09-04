#include <mbgl/renderer/sources/render_custom_geometry_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/tile/custom_geometry_tile.hpp>

namespace mbgl {

using namespace style;

RenderCustomGeometrySource::RenderCustomGeometrySource(Immutable<style::CustomGeometrySource::Impl> impl_,
                                                       const TaggedScheduler& threadPool_)
    : RenderTileSource(std::move(impl_), threadPool_) {
    tilePyramid.setObserver(this);
}

const style::CustomGeometrySource::Impl& RenderCustomGeometrySource::impl() const {
    return static_cast<const style::CustomGeometrySource::Impl&>(*baseImpl);
}

void RenderCustomGeometrySource::update(Immutable<style::Source::Impl> baseImpl_,
                                        const std::vector<Immutable<style::LayerProperties>>& layers,
                                        const bool needsRendering,
                                        const bool needsRelayout,
                                        const TileParameters& parameters) {
    bool didStartUpdate = false;

    if (baseImpl != baseImpl_) {
        std::swap(baseImpl, baseImpl_);

        // Clear tile pyramid only if updated source has different tile options,
        // zoom range or initialization state for a custom tile loader.
        auto newImpl = staticImmutableCast<style::CustomGeometrySource::Impl>(baseImpl);
        auto currentImpl = staticImmutableCast<style::CustomGeometrySource::Impl>(baseImpl_);
        if (*newImpl != *currentImpl) {
            didStartUpdate = true;
            onTilePyramidWillUpdate();

            tilePyramid.clearAll();
        }
    }

    enabled = needsRendering;

    auto tileLoader = impl().getTileLoader();
    if (!tileLoader) {
        if (didStartUpdate) {
            onTilePyramidUpdated();
        }
        return;
    }

    if (!didStartUpdate) {
        onTilePyramidWillUpdate();
    }

    tilePyramid.update(layers,
                       needsRendering,
                       needsRelayout,
                       parameters,
                       *baseImpl,
                       impl().getTileOptions()->tileSize,
                       impl().getZoomRange(),
                       {},
                       [&](const OverscaledTileID& tileID, TileObserver* observer_) {
                           return std::make_unique<CustomGeometryTile>(
                               tileID, impl().id, parameters, impl().getTileOptions(), *tileLoader, observer_);
                       });

    onTilePyramidUpdated();
}

} // namespace mbgl
