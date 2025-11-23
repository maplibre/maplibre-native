#include <mbgl/renderer/layers/render_color_relief_layer.hpp>
#include <mbgl/renderer/buckets/raster_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/sources/render_raster_dem_source.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/mat3.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>

namespace mbgl {

using namespace style;

inline const ColorReliefLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == ColorReliefLayer::Impl::staticTypeInfo());
    return static_cast<const ColorReliefLayer::Impl&>(*impl);
}

RenderColorReliefLayer::RenderColorReliefLayer(Immutable<style::ColorReliefLayer::Impl> _impl)
    : RenderLayer(makeMutable<ColorReliefLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paintProperties()) {}

RenderColorReliefLayer::~RenderColorReliefLayer() = default;

void RenderColorReliefLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paintProperties().transitioned(parameters, std::move(unevaluated));
}

void RenderColorReliefLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = staticImmutableCast<ColorReliefLayerProperties>(evaluatedProperties);
    
    auto evaluated_ = unevaluated.evaluate(parameters);

    passes = evaluated_.get<ColorReliefOpacity>() > 0.0f
        ? RenderPass::Translucent
        : RenderPass::None;

    properties->evaluated = std::move(evaluated_);
    properties->renderPasses = makeMutable<RenderPass>(passes);
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

void RenderColorReliefLayer::prepare(const LayerPrepareParameters&) {
    // TODO: Prepare color ramp textures
}

void RenderColorReliefLayer::render(PaintParameters&) {
    // TODO: Implement rendering
    // This will be implemented once shader infrastructure is in place
}

bool RenderColorReliefLayer::queryIntersectsFeature(
        const GeometryCoordinates&,
        const GeometryTileFeature&,
        float,
        const TransformState&,
        float,
        const mat4&,
        const FeatureState&) const {
    return false;
}

void RenderColorReliefLayer::updateColorRamp() {
    const auto& evaluated_ = staticImmutableCast<ColorReliefLayerProperties>(evaluatedProperties)->evaluated;
    const auto& colorRampValue = evaluated_.get<ColorReliefColor>();
    
    if (!colorRampValue.getValue()) {
        colorRampSize = 0;
        return;
    }

    // TODO: Parse expression and prepare color ramp data
    // For now, just set a default size
    colorRampSize = 256;
}

} // namespace mbgl
