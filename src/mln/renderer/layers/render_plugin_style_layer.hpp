#pragma once

#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>

namespace mln {

class RenderPluginStyleLayer final : public RenderLayer {
public:
    explicit RenderPluginStyleLayer(Immutable<style::PluginStyleLayer::Impl>);
    ~RenderPluginStyleLayer() override = default;

    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const PaintParameters&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;

    bool is3D() const override;

    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;

private:
    void transition(const TransitionParameters&) override {}
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override { return false; }
    bool hasCrossfade() const override { return false; }
    void markContextDestroyed() override { RenderLayer::markContextDestroyed(); }
};

} // namespace mln
