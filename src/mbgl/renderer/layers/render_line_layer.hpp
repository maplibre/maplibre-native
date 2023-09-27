#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>
#include <mbgl/programs/uniforms.hpp>
#include <mbgl/style/image_impl.hpp>
#include <mbgl/layout/pattern_layout.hpp>
#include <mbgl/gfx/texture.hpp>

#include <optional>
#include <memory>

namespace mbgl {

#if MLN_LEGACY_RENDERER
class LineProgram;
class LineGradientProgram;
class LineSDFProgram;
class LinePatternProgram;
#endif

#if MLN_DRAWABLE_RENDERER
namespace gfx {
class ShaderGroup;
class UniformBuffer;
using ShaderGroupPtr = std::shared_ptr<ShaderGroup>;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

class LineLayerTweaker;
using LineLayerTweakerPtr = std::shared_ptr<LineLayerTweaker>;
#endif

class RenderLineLayer final : public RenderLayer {
public:
    explicit RenderLineLayer(Immutable<style::LineLayer::Impl>);
    ~RenderLineLayer() override;

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
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void prepare(const LayerPrepareParameters&) override;
    void upload(gfx::UploadPass&) override;

#if MLN_LEGACY_RENDERER
    void render(PaintParameters&) override;
#endif

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
    std::optional<gfx::Texture> colorRampTexture;

#if MLN_DRAWABLE_RENDERER
    gfx::Texture2DPtr colorRampTexture2D;
#endif

#if MLN_LEGACY_RENDERER
    // Programs
    std::shared_ptr<LineProgram> lineProgram;
    std::shared_ptr<LineGradientProgram> lineGradientProgram;
    std::shared_ptr<LineSDFProgram> lineSDFProgram;
    std::shared_ptr<LinePatternProgram> linePatternProgram;
#endif
#if MLN_DRAWABLE_RENDERER
    gfx::ShaderGroupPtr lineShaderGroup;
    gfx::ShaderGroupPtr lineGradientShaderGroup;
    gfx::ShaderGroupPtr lineSDFShaderGroup;
    gfx::ShaderGroupPtr linePatternShaderGroup;

    gfx::DrawableTweakerPtr iconTweaker;
#endif
};

} // namespace mbgl
