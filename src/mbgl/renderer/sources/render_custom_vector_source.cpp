#include <mbgl/renderer/sources/render_custom_vector_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/tile/custom_vector_tile.hpp>
#include <mbgl/util/constants.hpp>

namespace mbgl {

using namespace style;

RenderCustomVectorSource::RenderCustomVectorSource(Immutable<style::CustomVectorSource::Impl> impl_,
                                                   const TaggedScheduler& threadPool_)
    : RenderTileSource(std::move(impl_), threadPool_) {
    tilePyramid.setObserver(this);
}

const style::CustomVectorSource::Impl& RenderCustomVectorSource::impl() const {
    return static_cast<const style::CustomVectorSource::Impl&>(*baseImpl);
}

void RenderCustomVectorSource::update(Immutable<style::Source::Impl> baseImpl_,
                                      const std::vector<Immutable<style::LayerProperties>>& layers,
                                      const bool needsRendering,
                                      const bool needsRelayout,
                                      const TileParameters& parameters) {
    if (baseImpl != baseImpl_) {
        std::swap(baseImpl, baseImpl_);

        auto newImpl = staticImmutableCast<style::CustomVectorSource::Impl>(baseImpl);
        auto currentImpl = staticImmutableCast<style::CustomVectorSource::Impl>(baseImpl_);
        if (*newImpl != *currentImpl) {
            tilePyramid.clearAll();
        }
    }

    enabled = needsRendering;

    auto tileLoader = impl().getTileLoader();
    if (!tileLoader) {
        return;
    }

    tilePyramid.update(layers,
                       needsRendering,
                       needsRelayout,
                       parameters,
                       *baseImpl,
                       util::tileSize_I,
                       impl().getZoomRange(),
                       {},
                       [&](const OverscaledTileID& tileID, TileObserver* observer_) {
                           return std::make_unique<CustomVectorTile>(
                               tileID, impl().id, parameters, *tileLoader, observer_);
                       });
}

} // namespace mbgl
