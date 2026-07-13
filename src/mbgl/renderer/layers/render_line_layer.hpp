#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>
#include <mbgl/shaders/uniforms.hpp>
#include <mbgl/style/image_impl.hpp>
#include <mbgl/layout/pattern_layout.hpp>

#include <optional>
#include <memory>

namespace mbgl {

namespace gfx {
class ShaderGroup;
class UniformBuffer;
using ShaderGroupPtr = std::shared_ptr<ShaderGroup>;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

class LineLayerTweaker;
using LineLayerTweakerPtr = std::shared_ptr<LineLayerTweaker>;

class RenderLineLayer final : public RenderLayer {
public:
    explicit RenderLineLayer(Immutable<style::LineLayer::Impl>);
    ~RenderLineLayer() override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void prepare(const LayerPrepareParameters&) override;

    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;

    // Paint properties
    style::LinePaintProperties::Unevaluated unevaluated;

    float getLineWidth(const GeometryTileFeature&, float, const FeatureState&) const;
    void updateColorRamp();

    std::shared_ptr<PremultipliedImage> colorRamp;
    gfx::Texture2DPtr colorRampTexture2D;

    gfx::ShaderGroupPtr lineShaderGroup;
    gfx::ShaderGroupPtr lineGradientShaderGroup;
    gfx::ShaderGroupPtr lineSDFShaderGroup;
    gfx::ShaderGroupPtr linePatternShaderGroup;
};

} // namespace mbgl
