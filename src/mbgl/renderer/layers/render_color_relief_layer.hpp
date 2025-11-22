#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/color_relief_layer_impl.hpp>
#include <mbgl/style/layers/color_relief_layer_properties.hpp>

namespace mbgl {

class PaintParameters;

class RenderColorReliefLayer final : public RenderLayer {
public:
    explicit RenderColorReliefLayer(Immutable<style::ColorReliefLayer::Impl>);
    ~RenderColorReliefLayer() override;

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void markContextDestroyed() override;

    void prepare(const LayerPrepareParameters&) override;

    void render(PaintParameters&) override;

    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;

    // Update color ramp textures from expression
    void updateColorRamp();

    // Paint properties
    style::ColorReliefPaintProperties::Unevaluated unevaluated;
    style::ColorReliefPaintProperties::PossiblyEvaluated evaluated;

    // Color ramp data - textures for elevation stops and colors
    std::optional<gfx::Texture> colorRampTexture;
    std::optional<gfx::Texture> elevationStopsTexture;
    int32_t colorRampSize = 0;
    bool colorRampChanged = true;
};

} // namespace mbgl
