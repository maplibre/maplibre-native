#pragma once

#include <mbgl/gfx/offscreen_texture.hpp>
#include <mbgl/renderer/buckets/heatmap_bucket.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/heatmap_layer_impl.hpp>
#include <mbgl/style/layers/heatmap_layer_properties.hpp>

#include <optional>

namespace mbgl {

class HeatmapLayerTweaker;
class HeatmapTextureLayerTweaker;
using HeatmapLayerTweakerPtr = std::shared_ptr<HeatmapLayerTweaker>;
using HeatmapTextureLayerTweakerPtr = std::shared_ptr<HeatmapTextureLayerTweaker>;

class RenderHeatmapLayer final : public RenderLayer {
public:
    explicit RenderHeatmapLayer(Immutable<style::HeatmapLayer::Impl>);
    ~RenderHeatmapLayer() override;

    void markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) override;

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
    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;
    void updateColorRamp();

    void layerChanged(const TransitionParameters& parameters,
                      const Immutable<style::Layer::Impl>& impl,
                      UniqueChangeRequestVec& changes) override;

    /// Remove all drawables for the tile from the layer group
    /// @return The number of drawables actually removed.
    std::size_t removeTile(RenderPass, const OverscaledTileID&) override;

    /// Remove all the drawables for tiles
    /// @return The number of drawables actually removed.
    std::size_t removeAllDrawables() override;

    // Paint properties
    style::HeatmapPaintProperties::Unevaluated unevaluated;
    std::shared_ptr<PremultipliedImage> colorRamp;
    std::unique_ptr<gfx::OffscreenTexture> renderTexture;
    SegmentVector segments;

    gfx::ShaderGroupPtr heatmapShaderGroup;
    gfx::ShaderProgramBasePtr heatmapTextureShader;
    RenderTargetPtr renderTarget;

    using TextureVertexVector = gfx::VertexVector<HeatmapTextureLayoutVertex>;
    std::shared_ptr<TextureVertexVector> sharedTextureVertices;

    // This is the layer tweaker for applying the off-screen texture to the framebuffer.
    // The inherited layer tweaker is for applying tiles to the off-screen texture.
    LayerTweakerPtr textureTweaker;
};

} // namespace mbgl
