#include <mln/annotation/render_annotation_source.hpp>
#include <mln/annotation/annotation_tile.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/util/instrumentation.hpp>

#include <mln/layermanager/layer_manager.hpp>

namespace mln {

using namespace style;

RenderAnnotationSource::RenderAnnotationSource(Immutable<AnnotationSource::Impl> impl_,
                                               const TaggedScheduler& threadPool_)
    : RenderTileSource(std::move(impl_), threadPool_) {
    assert(LayerManager::annotationsEnabled);
    tilePyramid.setObserver(this);
}

const AnnotationSource::Impl& RenderAnnotationSource::impl() const {
    return static_cast<const AnnotationSource::Impl&>(*baseImpl);
}

void RenderAnnotationSource::update(Immutable<style::Source::Impl> baseImpl_,
                                    const std::vector<Immutable<style::LayerProperties>>& layers,
                                    const bool needsRendering,
                                    const bool needsRelayout,
                                    const TileParameters& parameters) {
    MLN_TRACE_FUNC();

    std::swap(baseImpl, baseImpl_);

    enabled = needsRendering;

    tilePyramid.update(layers,
                       needsRendering,
                       needsRelayout,
                       parameters,
                       *baseImpl,
                       util::tileSize_I,
                       // Zoom level 16 is typically sufficient for annotations.
                       // See https://github.com/mapbox/mapbox-gl-native/issues/10197
                       {0, 16},
                       std::nullopt,
                       [&](const OverscaledTileID& tileID, TileObserver* observer_) {
                           return std::make_unique<AnnotationTile>(tileID, parameters, observer_);
                       });
}

std::unordered_map<std::string, std::vector<Feature>> RenderAnnotationSource::queryRenderedFeatures(
    const ScreenLineString& geometry,
    const TransformState& transformState,
    const std::unordered_map<std::string, const RenderLayer*>& layers,
    const RenderedQueryOptions& options,
    const mat4& projMatrix) const {
    return tilePyramid.queryRenderedFeatures(geometry, transformState, layers, options, projMatrix, {});
}

std::vector<Feature> RenderAnnotationSource::querySourceFeatures(const SourceQueryOptions&) const {
    return {};
}

} // namespace mln
