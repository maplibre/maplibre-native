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

class RenderLineLayer final : public RenderLayer {
public:
    explicit RenderLineLayer(Immutable<style::LineLayer::Impl>);
    ~RenderLineLayer() override;

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

    PremultipliedImage colorRamp;
    std::optional<gfx::Texture> colorRampTexture;

#if MLN_LEGACY_RENDERER
    // Programs
    std::shared_ptr<LineProgram> lineProgram;
    std::shared_ptr<LineGradientProgram> lineGradientProgram;
    std::shared_ptr<LineSDFProgram> lineSDFProgram;
    std::shared_ptr<LinePatternProgram> linePatternProgram;
#endif
};

} // namespace mbgl
