#include <mbgl/renderer/layers/render_color_relief_layer.hpp>
#include <mbgl/renderer/buckets/raster_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/sources/render_raster_dem_source.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/mat3.hpp>
#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

inline const ColorReliefLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == ColorReliefLayer::Impl::staticTypeInfo());
    return static_cast<const ColorReliefLayer::Impl&>(*impl);
}

RenderColorReliefLayer::RenderColorReliefLayer(Immutable<style::ColorReliefLayer::Impl> _impl)
    : RenderLayer(makeMutable<ColorReliefLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

RenderColorReliefLayer::~RenderColorReliefLayer() = default;

void RenderColorReliefLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
}

void RenderColorReliefLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = makeMutable<ColorReliefLayerProperties>(
        staticImmutableCast<ColorReliefLayer::Impl>(baseImpl),
        unevaluated.evaluate(parameters));

    auto& evaluated_ = properties->evaluated;
    passes = RenderPass::Translucent;

    // Check if color ramp changed
    const auto previousColorRamp = std::move(evaluated.get<ColorReliefColor>());
    if (previousColorRamp != evaluated_.get<ColorReliefColor>()) {
        colorRampChanged = true;
    }

    evaluated = std::move(evaluated_);
    properties->renderPasses = makeMutable<RenderLayerPasses>(passes);
    evaluatedProperties = std::move(properties);
}

bool RenderColorReliefLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderColorReliefLayer::hasCrossfade() const {
    return false;
}

void RenderColorReliefLayer::markContextDestroyed() {
    // Color ramp textures will be recreated as needed
}

void RenderColorReliefLayer::prepare(const LayerPrepareParameters& params) {
    renderTiles = params.source->getRenderTiles();
    
    if (colorRampChanged) {
        updateColorRamp();
        colorRampChanged = false;
    }
}

void RenderColorReliefLayer::updateColorRamp() {
    const auto& colorRampValue = evaluated.get<ColorReliefColor>();
    
    if (!colorRampValue.expression) {
        colorRampSize = 0;
        return;
    }

    // TODO: Parse expression and prepare color ramp data
    // For now, just set a default size
    colorRampSize = 256;
}

void RenderColorReliefLayer::render(PaintParameters& parameters) {
    if (renderTiles.empty()) return;

    const auto& evaluated_ = static_cast<const ColorReliefLayerProperties&>(*evaluatedProperties).evaluated;
    const float opacity = evaluated_.get<ColorReliefOpacity>();

    if (opacity <= 0.0f) return;

    // TODO: Implement rendering
    // This will require creating color ramp textures and setting up the rendering pipeline
    // similar to how hillshade rendering works
}

bool RenderColorReliefLayer::queryIntersectsFeature(const GeometryCoordinates&,
                                                     const GeometryTileFeature&,
                                                     float,
                                                     const TransformState&,
                                                     float,
                                                     const mat4&,
                                                     const FeatureState&) const {
    // Color relief layers don't support feature querying
    return false;
}

} // namespace mbgl
