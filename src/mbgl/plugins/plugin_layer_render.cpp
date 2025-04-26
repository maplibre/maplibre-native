#include "plugin_layer_render.hpp"

using namespace mbgl;

// RenderHeatmapLayer::RenderHeatmapLayer(Immutable<HeatmapLayer::Impl> _impl)
//     : RenderLayer(makeMutable<HeatmapLayerProperties>(std::move(_impl))),
//       unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
//     styleDependencies = unevaluated.getDependencies();
//     colorRamp = std::make_shared<PremultipliedImage>(Size(256, 1));
// }

RenderPluginLayer::RenderPluginLayer(Immutable<style::PluginLayer::Impl> _impl)
    : RenderLayer(makeMutable<style::PluginLayerProperties>(std::move(_impl))) {}

RenderPluginLayer::~RenderPluginLayer() = default;

void RenderPluginLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {}

/// Generate any changes needed by the layer
void RenderPluginLayer::update(gfx::ShaderRegistry&,
                               gfx::Context&,
                               const TransformState&,
                               const std::shared_ptr<UpdateParameters>&,
                               const RenderTree&,
                               UniqueChangeRequestVec&) {
    // TODO: What should be implemented here?
}

void RenderPluginLayer::prepare(const LayerPrepareParameters&) {
    // TODO: What should be implemented here?
}

// --- Private methods
void RenderPluginLayer::transition(const TransitionParameters&) {}

void RenderPluginLayer::evaluate(const PropertyEvaluationParameters&) {}

bool RenderPluginLayer::hasTransition() const {
    return false;
}

bool RenderPluginLayer::hasCrossfade() const {
    return false;
}
bool RenderPluginLayer::queryIntersectsFeature(const GeometryCoordinates&,
                                               const GeometryTileFeature&,
                                               float,
                                               const TransformState&,
                                               float,
                                               const mat4&,
                                               const FeatureState&) const {
    return false;
}

void RenderPluginLayer::layerChanged(const TransitionParameters& parameters,
                                     const Immutable<style::Layer::Impl>& impl,
                                     UniqueChangeRequestVec& changes) {}

/// Remove all drawables for the tile from the layer group
/// @return The number of drawables actually removed.
std::size_t RenderPluginLayer::removeTile(RenderPass, const OverscaledTileID&) {
    return 0;
}

/// Remove all the drawables for tiles
/// @return The number of drawables actually removed.
std::size_t RenderPluginLayer::removeAllDrawables() {
    return 0;
}
