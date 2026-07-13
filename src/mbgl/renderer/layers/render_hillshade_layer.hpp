#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/style/layers/hillshade_layer_impl.hpp>
#include <mbgl/style/layers/hillshade_layer_properties.hpp>
#include <mbgl/tile/tile_id.hpp>

namespace mbgl {

class HillshadeLayerTweaker;
using HillshadeLayerTweakerPtr = std::shared_ptr<HillshadeLayerTweaker>;

class RenderHillshadeLayer : public RenderLayer {
public:
    explicit RenderHillshadeLayer(Immutable<style::HillshadeLayer::Impl>);
    ~RenderHillshadeLayer() override;

    void markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) override;

    void layerRemoved(UniqueChangeRequestVec&) override;

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

    void updateLayerTweaker();

    void layerChanged(const TransitionParameters& parameters,
                      const Immutable<style::Layer::Impl>& impl,
                      UniqueChangeRequestVec& changes) override;

    void prepare(const LayerPrepareParameters&) override;

    void addRenderTarget(const RenderTargetPtr&, UniqueChangeRequestVec&);
    void removeRenderTargets(UniqueChangeRequestVec&);

    // Paint properties
    style::HillshadePaintProperties::Unevaluated unevaluated;
    uint8_t maxzoom = util::TERRAIN_RGB_MAXZOOM;

    std::array<float, 2> getLatRange(const UnwrappedTileID& id);
    std::array<float, 2> getLight(const PaintParameters& parameters);

    gfx::ShaderProgramBasePtr hillshadePrepareShader;
    gfx::ShaderProgramBasePtr hillshadeShader;
    std::vector<RenderTargetPtr> activatedRenderTargets;

    using HillshadeVertexVector = gfx::VertexVector<HillshadeLayoutVertex>;
    std::shared_ptr<HillshadeVertexVector> staticDataSharedVertices;

    LayerTweakerPtr prepareLayerTweaker;
};

} // namespace mbgl
