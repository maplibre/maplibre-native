#pragma once

#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>

#include <map>

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

    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    void layerChanged(const TransitionParameters&,
                      const Immutable<style::Layer::Impl>&,
                      UniqueChangeRequestVec&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override { return false; }

    std::map<std::string, style::PluginTransitioningPropertyValue> transitioningPaintProperties;
    style::PluginPropertyMap evaluatedPluginProperties;
};

} // namespace mln
