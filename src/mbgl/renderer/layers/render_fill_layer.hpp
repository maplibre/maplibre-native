#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/layout/pattern_layout.hpp>

#include <memory>

namespace mbgl {

class FillBucket;
class FillProgram;
class FillPatternProgram;
class FillOutlineProgram;
class FillOutlinePatternProgram;

#if MLN_DRAWABLE_RENDERER
class FillLayerTweaker;
using FillLayerTweakerPtr = std::shared_ptr<FillLayerTweaker>;
#endif // MLN_DRAWABLE_RENDERER

class RenderFillLayer final : public RenderLayer {
public:
    explicit RenderFillLayer(Immutable<style::FillLayer::Impl>);
    ~RenderFillLayer() override;

#if MLN_DRAWABLE_RENDERER
    std::size_t removeAllDrawables() override;
    std::size_t removeTile(RenderPass, const OverscaledTileID&) override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;
#endif

private:
    void prepare(const LayerPrepareParameters&) override;
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;

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
    style::FillPaintProperties::Unevaluated unevaluated;

#if MLN_LEGACY_RENDERER
    // Programs
    std::shared_ptr<FillProgram> fillProgram;
    std::shared_ptr<FillPatternProgram> fillPatternProgram;
    std::shared_ptr<FillOutlineProgram> fillOutlineProgram;
    std::shared_ptr<FillOutlinePatternProgram> fillOutlinePatternProgram;
#endif
#if MLN_DRAWABLE_RENDERER
    gfx::ShaderGroupPtr fillShaderGroup;
    gfx::ShaderGroupPtr outlineShaderGroup;
    gfx::ShaderGroupPtr patternShaderGroup;
    gfx::ShaderGroupPtr outlinePatternShaderGroup;

    LayerGroupBasePtr outlineLayerGroup;
    gfx::ShaderProgramBasePtr outlineShader;

    class OulineDrawableTweaker;

#endif
};

} // namespace mbgl
