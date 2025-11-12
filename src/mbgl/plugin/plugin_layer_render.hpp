#pragma once

#include <mbgl/gfx/offscreen_texture.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/plugin/plugin_layer_impl.hpp>
#include <mbgl/plugin/plugin_layer_properties.hpp>

#include <map>
#include <optional>

namespace mbgl {

class RenderPluginLayer final : public RenderLayer {
public:
    explicit RenderPluginLayer(Immutable<style::PluginLayer::Impl>);
    ~RenderPluginLayer() override;

    void markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;

    void prepare(const LayerPrepareParameters&) override;

    void upload(gfx::UploadPass&) override;

    void setRenderFunction(style::PluginLayer::OnRenderLayer renderFunction) { _renderFunction = renderFunction; }
    void setUpdateFunction(style::PluginLayer::OnUpdateLayer updateFunction) { _updateFunction = updateFunction; }
    void setUpdatePropertiesFunction(style::PluginLayer::OnUpdateLayerProperties updateLayerPropertiesFunction) {
        _updateLayerPropertiesFunction = updateLayerPropertiesFunction;
    }

    void callRenderFunction(PaintParameters& paintParameters);

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;
    void updateColorRamp();

    void layerChanged(const TransitionParameters& parameters,
                      const Immutable<style::Layer::Impl>& impl,
                      UniqueChangeRequestVec& changes) override;

    /// Remove all drawables for the tile from the layer group
    /// @return The number of drawables actually removed.
    std::size_t removeTile(RenderPass, const OverscaledTileID&) override;

    /// Remove all the drawables for tiles
    /// @return The number of drawables actually removed.
    std::size_t removeAllDrawables() override;

    // The render methods
    style::PluginLayer::OnRenderLayer _renderFunction = nullptr;

    style::PluginLayer::OnUpdateLayer _updateFunction = nullptr;

    style::PluginLayer::OnUpdateLayerProperties _updateLayerPropertiesFunction = nullptr;

    std::map<OverscaledTileID, std::shared_ptr<mbgl::plugin::FeatureCollection>> _featureCollectionByTile;
};

} // namespace mbgl
