#pragma once

#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/circle_layer_impl.hpp>
#include <mln/style/layers/circle_layer_properties.hpp>

namespace mln {

class CircleLayerTweaker;
using CircleLayerTweakerPtr = std::shared_ptr<CircleLayerTweaker>;

class RenderCircleLayer final : public RenderLayer {
public:
    explicit RenderCircleLayer(Immutable<style::CircleLayer::Impl>);
    ~RenderCircleLayer() final = default;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry &,
                gfx::Context &,
                const TransformState &,
                const std::shared_ptr<UpdateParameters> &,
                const PaintParameters &,
                const RenderTree &,
                UniqueChangeRequestVec &) override;

private:
    void transition(const TransitionParameters &) override;
    void evaluate(const PropertyEvaluationParameters &) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;

    bool queryIntersectsFeature(const GeometryCoordinates &,
                                const GeometryTileFeature &,
                                float,
                                const TransformState &,
                                float,
                                const mat4 &,
                                const FeatureState &) const override;

private:
    // Paint properties
    style::CirclePaintProperties::Unevaluated unevaluated;

    gfx::ShaderGroupPtr circleShaderGroup;
};

} // namespace mln
