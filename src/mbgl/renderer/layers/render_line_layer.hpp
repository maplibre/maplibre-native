#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>
#include <mbgl/programs/uniforms.hpp>
#include <mbgl/style/image_impl.hpp>
#include <mbgl/layout/pattern_layout.hpp>
#include <mbgl/gfx/texture.hpp>

namespace mbgl {

class LineProgram;
class LineGradientProgram;
class LineSDFProgram;
class LinePatternProgram;

namespace gfx {
class ShaderGroup;
class UniformBuffer;
using ShaderGroupPtr = std::shared_ptr<ShaderGroup>;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

class RenderLineLayer final : public RenderLayer {
public:
    explicit RenderLineLayer(Immutable<style::LineLayer::Impl>);
    ~RenderLineLayer() override;

    void layerRemoved(UniqueChangeRequestVec&) override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void prepare(const LayerPrepareParameters&) override;
    void upload(gfx::UploadPass&) override;
    void render(PaintParameters&) override;

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

    PremultipliedImage colorRamp;
    std::optional<gfx::Texture> colorRampTexture;

    /// Remove all drawables for the tile from the layer group
    void removeTile(RenderPass, const OverscaledTileID&);

    // Programs
    std::shared_ptr<LineProgram> lineProgram;
    std::shared_ptr<LineGradientProgram> lineGradientProgram;
    std::shared_ptr<LineSDFProgram> lineSDFProgram;
    std::shared_ptr<LinePatternProgram> linePatternProgram;

    gfx::ShaderGroupPtr lineShaderGroup;
    gfx::ShaderGroupPtr lineGradientShaderGroup;
    gfx::ShaderGroupPtr lineSDFShaderGroup;
    gfx::ShaderGroupPtr linePatternShaderGroup;
};

} // namespace mbgl
