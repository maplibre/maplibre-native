#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/renderer/buckets/raster_bucket.hpp>
#include <mbgl/style/layers/raster_layer_impl.hpp>
#include <mbgl/style/layers/raster_layer_properties.hpp>
#include <mbgl/gfx/context.hpp>

namespace mbgl {

class ImageSourceRenderData;

class RenderRasterLayer final : public RenderLayer {
public:
    explicit RenderRasterLayer(Immutable<style::RasterLayer::Impl>);
    ~RenderRasterLayer() override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;

protected:
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

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void prepare(const LayerPrepareParameters&) override;

    // Paint properties
    style::RasterPaintProperties::Unevaluated unevaluated;
    const ImageSourceRenderData* imageData = nullptr;

    gfx::ShaderProgramBasePtr rasterShader;
    LayerGroupPtr imageLayerGroup;

    using RasterVertexVector = gfx::VertexVector<RasterLayoutVertex>;
    using RasterVertexVectorPtr = std::shared_ptr<RasterVertexVector>;
    RasterVertexVectorPtr staticDataVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    using TriangleIndexVectorPtr = std::shared_ptr<TriangleIndexVector>;
    TriangleIndexVectorPtr staticDataIndices;

    using RasterSegmentVector = SegmentVector;
    using RasterSegmentVectorPtr = std::shared_ptr<RasterSegmentVector>;
    std::shared_ptr<RasterSegmentVector> staticDataSegments;
};

} // namespace mbgl
