#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/programs/hillshade_program.hpp>
#include <mbgl/programs/hillshade_prepare_program.hpp>
#include <mbgl/style/layers/hillshade_layer_impl.hpp>
#include <mbgl/style/layers/hillshade_layer_properties.hpp>
#include <mbgl/tile/tile_id.hpp>

namespace mbgl {

#if MLN_DRAWABLE_RENDERER
class HillshadeLayerTweaker;
using HillshadeLayerTweakerPtr = std::shared_ptr<HillshadeLayerTweaker>;
#endif // MLN_DRAWABLE_RENDERER

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
                const std::shared_ptr<UpdateParameters>&,
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

#if MLN_DRAWABLE_RENDERER
    void updateLayerTweaker();

    void layerChanged(const TransitionParameters& parameters,
                      const Immutable<style::Layer::Impl>& impl,
                      UniqueChangeRequestVec& changes);

#endif // MLN_DRAWABLE_RENDERER

    void prepare(const LayerPrepareParameters&) override;

#if MLN_DRAWABLE_RENDERER
    void addRenderTarget(const RenderTargetPtr&, UniqueChangeRequestVec&);
    void removeRenderTargets(UniqueChangeRequestVec&);
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
    std::optional<int> hillshadeImageLocation;
    std::vector<RenderTargetPtr> activatedRenderTargets;

    using HillshadeVertexVector = gfx::VertexVector<HillshadeLayoutVertex>;
    std::shared_ptr<HillshadeVertexVector> staticDataSharedVertices;

    LayerTweakerPtr prepareLayerTweaker;
#endif
};

} // namespace mbgl
