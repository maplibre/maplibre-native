#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/location_indicator_layer.hpp>
#include <mbgl/style/layers/location_indicator_layer_impl.hpp>
#include <mbgl/style/layers/location_indicator_layer_properties.hpp>

namespace mbgl {
class RenderLocationIndicatorImpl;
class RenderLocationIndicatorLayer final : public RenderLayer {
public:
    explicit RenderLocationIndicatorLayer(Immutable<style::LocationIndicatorLayer::Impl>);
    ~RenderLocationIndicatorLayer() override;

#if MLN_RENDER_BACKEND_VULKAN
    void update(gfx::ShaderRegistry &,
                gfx::Context &,
                const TransformState &,
                const std::shared_ptr<UpdateParameters> &,
                const RenderTree &,
                UniqueChangeRequestVec &) override;
#endif

private:
    void transition(const TransitionParameters &) override;
    void evaluate(const PropertyEvaluationParameters &) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void markContextDestroyed() override;
    void prepare(const LayerPrepareParameters &) override;

#if MLN_RENDER_BACKEND_OPENGL
    void render(PaintParameters &) override;
#endif

    void populateDynamicRenderFeatureIndex(DynamicFeatureIndex &) const override;

private:
    bool contextDestroyed = false;
    std::unique_ptr<RenderLocationIndicatorImpl> renderImpl;
    style::LocationIndicatorPaintProperties::Unevaluated unevaluated;

#if MLN_DRAWABLE_RENDERER
    // Drawable shaders
    gfx::ShaderProgramBasePtr quadShader;
    gfx::ShaderProgramBasePtr circleShader;
#endif
};

} // namespace mbgl
