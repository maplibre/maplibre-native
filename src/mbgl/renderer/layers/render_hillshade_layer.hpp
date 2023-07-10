#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/programs/hillshade_program.hpp>
#include <mbgl/programs/hillshade_prepare_program.hpp>
#include <mbgl/style/layers/hillshade_layer_impl.hpp>
#include <mbgl/style/layers/hillshade_layer_properties.hpp>
#include <mbgl/tile/tile_id.hpp>

namespace mbgl {

class HillshadeProgram;
class HillshadePrepareProgram;

class RenderHillshadeLayer : public RenderLayer {
public:
    explicit RenderHillshadeLayer(Immutable<style::HillshadeLayer::Impl>);
    ~RenderHillshadeLayer() override;

#if MLN_DRAWABLE_RENDERER
    void markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) override;

    void layerRemoved(UniqueChangeRequestVec&) override;

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

#if MLN_LEGACY_RENDERER
    void render(PaintParameters&) override;
#endif

    void prepare(const LayerPrepareParameters&) override;

#if MLN_DRAWABLE_RENDERER
    /// Remove render target for the tile
    void removeRenderTarget(const OverscaledTileID&, UniqueChangeRequestVec&);

    /// Remove all the render targets
    void removeAllRenderTargets(UniqueChangeRequestVec&);
#endif

    // Paint properties
    style::HillshadePaintProperties::Unevaluated unevaluated;
    uint8_t maxzoom = util::TERRAIN_RGB_MAXZOOM;

    std::array<float, 2> getLatRange(const UnwrappedTileID& id);
    std::array<float, 2> getLight(const PaintParameters& parameters);

#if MLN_LEGACY_RENDERER
    // Programs
    std::shared_ptr<HillshadeProgram> hillshadeProgram;
    std::shared_ptr<HillshadePrepareProgram> hillshadePrepareProgram;
#endif

#if MLN_DRAWABLE_RENDERER
    gfx::ShaderProgramBasePtr hillshadePrepareShader;
    gfx::ShaderProgramBasePtr hillshadeShader;
    std::unordered_map<OverscaledTileID, RenderTargetPtr> renderTargets;
    
    using HillshadeVertexVector = gfx::VertexVector<HillshadeLayoutVertex>;
    std::shared_ptr<HillshadeVertexVector> staticDataSharedVertices;
#endif
};

} // namespace mbgl
