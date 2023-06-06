#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/raster_layer_impl.hpp>
#include <mbgl/style/layers/raster_layer_properties.hpp>

namespace mbgl {

class ImageSourceRenderData;
class RasterProgram;

class RenderRasterLayer final : public RenderLayer {
public:
    explicit RenderRasterLayer(Immutable<style::RasterLayer::Impl>);
    ~RenderRasterLayer() override;

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
    void render(PaintParameters&) override;

    /// Remove all drawables for the tile from the layer group
    void removeTile(RenderPass, const OverscaledTileID&);

    // Paint properties
    style::RasterPaintProperties::Unevaluated unevaluated;
    const ImageSourceRenderData* imageData = nullptr;

    // Programs
    std::shared_ptr<RasterProgram> rasterProgram;

    gfx::ShaderProgramBasePtr rasterShader;
};

} // namespace mbgl
