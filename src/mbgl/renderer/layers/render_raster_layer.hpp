#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/raster_layer_impl.hpp>
#include <mbgl/style/layers/raster_layer_properties.hpp>
#include <mbgl/gfx/context.hpp>

namespace mbgl {

class ImageSourceRenderData;
class RasterProgram;

class RenderRasterLayer final : public RenderLayer {
public:
    explicit RenderRasterLayer(Immutable<style::RasterLayer::Impl>);
    ~RenderRasterLayer() override;

#if MLN_DRAWABLE_RENDERER
    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;
#endif

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void prepare(const LayerPrepareParameters&) override;

#if MLN_LEGACY_RENDERER
    void render(PaintParameters&) override;
#endif

    // Paint properties
    style::RasterPaintProperties::Unevaluated unevaluated;
    const ImageSourceRenderData* imageData = nullptr;

#if MLN_LEGACY_RENDERER
    // Programs
    std::shared_ptr<RasterProgram> rasterProgram;
#endif

#if MLN_DRAWABLE_RENDERER
    gfx::ShaderProgramBasePtr rasterShader;
    LayerGroupPtr imageLayerGroup;
#endif
};

} // namespace mbgl
