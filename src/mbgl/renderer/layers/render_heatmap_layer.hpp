#pragma once

#include <mbgl/gfx/offscreen_texture.hpp>
#include <mbgl/gfx/texture.hpp>
#include <mbgl/programs/heatmap_program.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/heatmap_layer_impl.hpp>
#include <mbgl/style/layers/heatmap_layer_properties.hpp>

#include <optional>

namespace mbgl {

class RenderHeatmapLayer final : public RenderLayer {
public:
    explicit RenderHeatmapLayer(Immutable<style::HeatmapLayer::Impl>);
    ~RenderHeatmapLayer() override;

    void layerRemoved(UniqueChangeRequestVec&) override;
    void layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec& changes) override;
    void markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) override;

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
    void upload(gfx::UploadPass&) override;
    void render(PaintParameters&) override;

    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;
    void updateColorRamp();

    /// Remove all drawables for the tile from the layer group
    void removeTile(RenderPass, const OverscaledTileID&);

    // Paint properties
    style::HeatmapPaintProperties::Unevaluated unevaluated;
    std::shared_ptr<PremultipliedImage> colorRamp;
    std::unique_ptr<gfx::OffscreenTexture> renderTexture;
    std::optional<gfx::Texture> colorRampTexture;
    SegmentVector<HeatmapTextureAttributes> segments;

    // Programs
    std::shared_ptr<HeatmapProgram> heatmapProgram;
    std::shared_ptr<HeatmapTextureProgram> heatmapTextureProgram;

    gfx::ShaderGroupPtr heatmapShaderGroup;
    gfx::ShaderProgramBasePtr heatmapTextureShader;
    RenderTargetPtr renderTarget;
    LayerGroupPtr textureLayerGroup;
};

} // namespace mbgl
