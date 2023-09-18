#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/programs/raster_program.hpp>
#include <mbgl/style/layers/raster_layer_impl.hpp>
#include <mbgl/style/layers/raster_layer_properties.hpp>
#include <mbgl/gfx/context.hpp>

namespace mbgl {

#if MLN_DRAWABLE_RENDERER
class RasterLayerTweaker;
using RasterLayerTweakerPtr = std::shared_ptr<RasterLayerTweaker>;
#endif // MLN_DRAWABLE_RENDERER

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
                const std::shared_ptr<UpdateParameters>&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;
#endif

protected:
#if MLN_DRAWABLE_RENDERER
    /// @brief Called by the RenderOrchestrator during RenderTree construction.
    /// This event is run to indicate if the layer should render or not for the current frame.
    /// @param willRender Indicates if this layer should render or not
    /// @param changes The collection of current pending change requests
    void markLayerRenderable(bool willRender, UniqueChangeRequestVec&) override;

    /// @brief Called when the layer index changes
    /// This event is run when a layer is added or removed from the style.
    /// @param newLayerIndex The new layer index for this layer
    /// @param changes The collection of current pending change requests
    void layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec&) override;

    /// Called when the style layer is removed
    void layerRemoved(UniqueChangeRequestVec&) override;
#endif // MLN_DRAWABLE_RENDERER

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void prepare(const LayerPrepareParameters&) override;

#if MLN_LEGACY_RENDERER
    void render(PaintParameters&) override;
#endif

#if MLN_DRAWABLE_RENDERER
    void updateLayerTweaker();
#endif // MLN_DRAWABLE_RENDERER

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

    using RasterVertexVector = gfx::VertexVector<RasterLayoutVertex>;
    using RasterVertexVectorPtr = std::shared_ptr<RasterVertexVector>;
    RasterVertexVectorPtr staticDataVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    using TriangleIndexVectorPtr = std::shared_ptr<TriangleIndexVector>;
    TriangleIndexVectorPtr staticDataIndices;

    using RasterSegmentVector = SegmentVector<RasterAttributes>;
    using RasterSegmentVectorPtr = std::shared_ptr<RasterSegmentVector>;
    std::shared_ptr<RasterSegmentVector> staticDataSegments;

    RasterLayerTweakerPtr tweaker;
    bool overdrawInspector = false;
#endif
};

} // namespace mbgl
