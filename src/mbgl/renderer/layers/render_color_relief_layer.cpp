#include <mbgl/renderer/layers/render_color_relief_layer.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/style/layers/color_relief_layer_impl.hpp>

namespace mbgl {

using namespace style;

namespace {

inline const ColorReliefLayer::Impl& impl_cast(const Immutable<Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == ColorReliefLayer::Impl::staticTypeInfo());
    return static_cast<const ColorReliefLayer::Impl&>(*impl);
}

} // namespace

RenderColorReliefLayer::RenderColorReliefLayer(Immutable<ColorReliefLayer::Impl> _impl)
    : RenderLayer(makeMutable<ColorReliefLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

RenderColorReliefLayer::~RenderColorReliefLayer() = default;

void RenderColorReliefLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
}

void RenderColorReliefLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto previousProperties = staticImmutableCast<ColorReliefLayerProperties>(evaluatedProperties);
    auto properties = makeMutable<ColorReliefLayerProperties>(
        staticImmutableCast<ColorReliefLayer::Impl>(baseImpl),
        unevaluated.evaluate(parameters, previousProperties->evaluated));

    passes = (properties->evaluated.get<style::ColorReliefOpacity>() > 0) 
        ? RenderPass::Translucent
        : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);
}

bool RenderColorReliefLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderColorReliefLayer::hasCrossfade() const {
    return false;
}

void RenderColorReliefLayer::markContextDestroyed() {
    // Nothing to clean up yet
}

void RenderColorReliefLayer::prepare(const LayerPrepareParameters&) {
    // TODO: Prepare rendering resources
}

void RenderColorReliefLayer::render(PaintParameters&) {
    // TODO: Implement rendering
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
    // TODO: Implement color ramp texture generation
    // Similar to heatmap's updateColorRamp()
}

} // namespace mbgl
