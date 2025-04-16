#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/programs/fill_program.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>

/**
    Control how the fill outlines are being generated:
    MLN_TRIANGULATE_FILL_OUTLINES = 0 : Simple line primitives will be generated. Draw using gfx::Lines
    MLN_TRIANGULATE_FILL_OUTLINES = 1 : Generate triangulated lines. Draw using gfx::Triangles and a Line shader.
 */
#define MLN_TRIANGULATE_FILL_OUTLINES (MLN_RENDER_BACKEND_METAL)

#if MLN_TRIANGULATE_FILL_OUTLINES
#include <mbgl/programs/line_program.hpp>
#endif

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
    using LineVertexVector = gfx::VertexVector<LineLayoutVertex>;
    const std::shared_ptr<LineVertexVector> sharedLineVertices = std::make_shared<LineVertexVector>();
    LineVertexVector& lineVertices = *sharedLineVertices;

    using LineIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<LineIndexVector> sharedLineIndexes = std::make_shared<LineIndexVector>();
    LineIndexVector& lineIndexes = *sharedLineIndexes;

    SegmentVector<LineAttributes> lineSegments;
#endif // MLN_TRIANGULATE_FILL_OUTLINES

    using BasicLineIndexVector = gfx::IndexVector<gfx::Lines>;
    const std::shared_ptr<BasicLineIndexVector> sharedBasicLineIndexes = std::make_shared<BasicLineIndexVector>();
    BasicLineIndexVector& basicLines = *sharedBasicLineIndexes;

    SegmentVector<FillAttributes> basicLineSegments;

    using VertexVector = gfx::VertexVector<FillLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector<FillAttributes> triangleSegments;

    std::map<std::string, FillProgram::Binders> paintPropertyBinders;
};

} // namespace mbgl
