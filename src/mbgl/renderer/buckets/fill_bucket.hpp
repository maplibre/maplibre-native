#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/programs/fill_program.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>

#if MLN_DRAWABLE_RENDERER
/**
    Control how the fill outlines are being generated:
    MLN_TRIANGULATE_FILL_OUTLINES = 0 : Basic line primitives will be generated. Draw using gfx::Lines
    MLN_TRIANGULATE_FILL_OUTLINES = 1 : Additionally generate triangulated lines. Draw using gfx::Triangles and a Line
   shader. Pattern fill outlines are still drawn as basic lines.
 */
#define MLN_TRIANGULATE_FILL_OUTLINES 1
#else // MLN_DRAWABLE_RENDERER
// The Legacy Renderer is incompatible with triangulated lines
#define MLN_TRIANGULATE_FILL_OUTLINES 0
#endif // MLN_DRAWABLE_RENDERER

#if MLN_TRIANGULATE_FILL_OUTLINES
#include <mbgl/programs/line_program.hpp>
#endif

#include <vector>

namespace mbgl {

class BucketParameters;
class RenderFillLayer;

class FillBucket final : public Bucket {
public:
    ~FillBucket() override;
    using PossiblyEvaluatedLayoutProperties = style::FillLayoutProperties::PossiblyEvaluated;

    FillBucket(const PossiblyEvaluatedLayoutProperties& layout,
               const std::map<std::string, Immutable<style::LayerProperties>>& layerPaintProperties,
               float zoom,
               uint32_t overscaling);

    void addFeature(const GeometryTileFeature&,
                    const GeometryCollection&,
                    const mbgl::ImagePositions&,
                    const PatternLayerMap&,
                    std::size_t,
                    const CanonicalTileID&) override;

    bool hasData() const override;

    void upload(gfx::UploadPass&) override;

    float getQueryRadius(const RenderLayer&) const override;

    void update(const FeatureStates&, const GeometryTileLayer&, const std::string&, const ImagePositions&) override;

#if MLN_TRIANGULATE_FILL_OUTLINES
    /// Triangulated lines vertices
    using LineVertexVector = gfx::VertexVector<LineLayoutVertex>;
    const std::shared_ptr<LineVertexVector> sharedLineVertices = std::make_shared<LineVertexVector>();
    LineVertexVector& lineVertices = *sharedLineVertices;

    /// Triangulated lines indexes
    using LineIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<LineIndexVector> sharedLineIndexes = std::make_shared<LineIndexVector>();
    LineIndexVector& lineIndexes = *sharedLineIndexes;

    /// Triangulated lines segments
    SegmentVector<LineAttributes> lineSegments;

    /// for each lineVertex index, store the correspondent fillVertex index
    std::vector<std::size_t> lineToFillVertexIndex;
#endif // MLN_TRIANGULATE_FILL_OUTLINES

    /// Basic lines indexes
    using BasicLineIndexVector = gfx::IndexVector<gfx::Lines>;
    const std::shared_ptr<BasicLineIndexVector> sharedBasicLineIndexes = std::make_shared<BasicLineIndexVector>();
    BasicLineIndexVector& basicLines = *sharedBasicLineIndexes;

    /// Basic lines segments
    SegmentVector<FillAttributes> basicLineSegments;

    /// Fill vertices
    using VertexVector = gfx::VertexVector<FillLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    /// Fill indexes
    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    /// Fill segments
    SegmentVector<FillAttributes> triangleSegments;

#if MLN_LEGACY_RENDERER
    std::optional<gfx::VertexBuffer<FillLayoutVertex>> vertexBuffer;
    std::optional<gfx::IndexBuffer> lineIndexBuffer;
    std::optional<gfx::IndexBuffer> triangleIndexBuffer;
#endif // MLN_LEGACY_RENDERER

    std::map<std::string, FillProgram::Binders> paintPropertyBinders;
};

} // namespace mbgl
