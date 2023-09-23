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

#if MLN_DRAWABLE_RENDERER
    void markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;
#endif

private:
    void prepare(const LayerPrepareParameters&) override;
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
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
    void updateColorRamp();

#if MLN_DRAWABLE_RENDERER
    /// Remove all drawables for the tile from the layer group
    void removeTile(RenderPass, const OverscaledTileID&) override;

    /// Remove all the drawables for tiles
    void removeAllDrawables() override;
#endif

    // Paint properties
    style::HeatmapPaintProperties::Unevaluated unevaluated;
    std::shared_ptr<PremultipliedImage> colorRamp;
    std::unique_ptr<gfx::OffscreenTexture> renderTexture;
    std::optional<gfx::Texture> colorRampTexture;
    SegmentVector<HeatmapTextureAttributes> segments;

#if MLN_LEGACY_RENDERER
    // Programs
    std::shared_ptr<HeatmapProgram> heatmapProgram;
    std::shared_ptr<HeatmapTextureProgram> heatmapTextureProgram;
#endif

#if MLN_DRAWABLE_RENDERER
    gfx::ShaderGroupPtr heatmapShaderGroup;
    gfx::ShaderProgramBasePtr heatmapTextureShader;
    RenderTargetPtr renderTarget;

    using TextureVertexVector = gfx::VertexVector<HeatmapTextureLayoutVertex>;
    std::shared_ptr<TextureVertexVector> sharedTextureVertices;
#endif
};

} // namespace mbgl
