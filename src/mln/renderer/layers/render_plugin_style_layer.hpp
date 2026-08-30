#pragma once

#include <mln/renderer/buckets/hillshade_bucket.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>

#include <map>

namespace mln {

class RenderPluginStyleLayer final : public RenderLayer {
public:
    explicit RenderPluginStyleLayer(Immutable<style::PluginStyleLayer::Impl>);
    ~RenderPluginStyleLayer() override = default;

    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const PaintParameters&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;

    bool is3D() const override;

    void prepare(const LayerPrepareParameters&) override;
    void markLayerRenderable(bool, UniqueChangeRequestVec&) override;
    void layerRemoved(UniqueChangeRequestVec&) override;

    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;

private:
    void transition(const TransitionParameters&) override {}
    void evaluate(const PropertyEvaluationParameters&) override;
    void layerChanged(const TransitionParameters&,
                      const Immutable<style::Layer::Impl>&,
                      UniqueChangeRequestVec&) override;
    bool hasTransition() const override { return false; }
    bool hasCrossfade() const override { return false; }
    void markContextDestroyed() override;

    void updateRasterDEMGraph(gfx::ShaderRegistry&,
                              gfx::Context&,
                              const PaintParameters&,
                              UniqueChangeRequestVec&);
    void addRenderTarget(const RenderTargetPtr&, UniqueChangeRequestVec&);
    void removeRenderTargets(UniqueChangeRequestVec&);

    struct RasterGraphTileState {
        util::SimpleIdentity bucketID = util::SimpleIdentity::Empty;
        uint64_t demRevision = 0;
        uint64_t maskRevision = 0;
        gfx::Texture2DPtr sourceTexture;
        std::map<uint32_t, RenderTargetPtr> renderTargets;
    };

    uint8_t sourceMaxZoom = util::TERRAIN_RGB_MAXZOOM;
    std::map<OverscaledTileID, RasterGraphTileState> rasterGraphTiles;
    std::vector<RenderTargetPtr> activatedRenderTargets;
    std::shared_ptr<gfx::VertexVector<HillshadeLayoutVertex>> rasterSharedVertices;
};

} // namespace mln
