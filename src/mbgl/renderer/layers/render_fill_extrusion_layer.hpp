#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_impl.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_properties.hpp>
#include <mbgl/gfx/offscreen_texture.hpp>

namespace mbgl {

#if MLN_LEGACY_RENDERER
class FillExtrusionProgram;
class FillExtrusionPatternProgram;
#endif // MLN_LEGACY_RENDERER

class RenderFillExtrusionLayer final : public RenderLayer {
public:
    explicit RenderFillExtrusionLayer(Immutable<style::FillExtrusionLayer::Impl>);
    ~RenderFillExtrusionLayer() override;

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    bool is3D() const override;

#if MLN_LEGACY_RENDERER
    void render(PaintParameters&) override;
#endif

#if MLN_DRAWABLE_RENDERER
    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;
#endif // MLN_DRAWABLE_RENDERER

    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;

    // Paint properties
    style::FillExtrusionPaintProperties::Unevaluated unevaluated;

#if MLN_LEGACY_RENDERER
    // Programs
    std::shared_ptr<FillExtrusionProgram> fillExtrusionProgram;
    std::shared_ptr<FillExtrusionPatternProgram> fillExtrusionPatternProgram;
#endif

#if MLN_DRAWABLE_RENDERER
    gfx::ShaderGroupPtr fillExtrusionGroup;
    gfx::ShaderGroupPtr fillExtrusionPatternGroup;
#endif // MLN_DRAWABLE_RENDERER

    std::unique_ptr<gfx::OffscreenTexture> renderTexture;
};

} // namespace mbgl
