#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/programs/fill_program.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>

#if MLN_DRAWABLE_RENDERER
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

#if MLN_DRAWABLE_RENDERER
    using LineVertexVector = gfx::VertexVector<LineLayoutVertex>;
    const std::shared_ptr<LineVertexVector> sharedLineVertices = std::make_shared<LineVertexVector>();
    LineVertexVector& lineVertices = *sharedLineVertices;

    using LineIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<LineIndexVector> sharedLineIndexes = std::make_shared<LineIndexVector>();
    LineIndexVector& lineIndexes = *sharedLineIndexes;

    SegmentVector<LineAttributes> lineSegments;
#endif

    using VertexVector = gfx::VertexVector<FillLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector<FillAttributes> triangleSegments;

#if MLN_LEGACY_RENDERER
    using LineIndexVector = gfx::IndexVector<gfx::Lines>;
    const std::shared_ptr<LineIndexVector> sharedLines = std::make_shared<LineIndexVector>();
    LineIndexVector& lines = *sharedLines;

    SegmentVector<FillAttributes> lineSegments;

    std::optional<gfx::VertexBuffer<FillLayoutVertex>> vertexBuffer;
    std::optional<gfx::IndexBuffer> lineIndexBuffer;
    std::optional<gfx::IndexBuffer> triangleIndexBuffer;
#endif // MLN_LEGACY_RENDERER

    std::map<std::string, FillProgram::Binders> paintPropertyBinders;
};

} // namespace mbgl
