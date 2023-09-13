#pragma once

#include <mbgl/style/layers/custom_layer_impl.hpp>
#include <mbgl/renderer/render_layer.hpp>

namespace mbgl {

class RenderCustomLayer final : public RenderLayer {
public:
    explicit RenderCustomLayer(Immutable<style::CustomLayer::Impl>);
    ~RenderCustomLayer() override;

#if MLN_DRAWABLE_RENDERER
    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;
#endif

private:
    void transition(const TransitionParameters&) override {}
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void markContextDestroyed() override;
    void prepare(const LayerPrepareParameters&) override;

#if MLN_LEGACY_RENDERER
    void render(PaintParameters&) override;
#endif

    bool contextDestroyed = false;
    std::shared_ptr<style::CustomLayerHost> host;
};

} // namespace mbgl
