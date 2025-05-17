#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>

/**
    Control how the fill outlines are being generated:
    MLN_TRIANGULATE_FILL_OUTLINES = 0 : Simple line primitives will be generated. Draw using gfx::Lines
    MLN_TRIANGULATE_FILL_OUTLINES = 1 : Generate triangulated lines. Draw using gfx::Triangles and a Line shader.
 */
#define MLN_TRIANGULATE_FILL_OUTLINES (MLN_RENDER_BACKEND_METAL)

#if MLN_TRIANGULATE_FILL_OUTLINES
#include <mbgl/renderer/buckets/line_bucket.hpp>
#endif

namespace mbgl {

class BucketParameters;
class RenderFillLayer;

using FillBinders = PaintPropertyBinders<style::FillPaintProperties::DataDrivenProperties>;
using FillLayoutVertex = gfx::Vertex<TypeList<attributes::pos>>;

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

    static FillLayoutVertex layoutVertex(Point<int16_t> p) { return FillLayoutVertex{{{p.x, p.y}}}; }

#if MLN_TRIANGULATE_FILL_OUTLINES
    using LineVertexVector = gfx::VertexVector<LineLayoutVertex>;
    const std::shared_ptr<LineVertexVector> sharedLineVertices = std::make_shared<LineVertexVector>();
    LineVertexVector& lineVertices = *sharedLineVertices;

    using LineIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<LineIndexVector> sharedLineIndexes = std::make_shared<LineIndexVector>();
    LineIndexVector& lineIndexes = *sharedLineIndexes;

    SegmentVector lineSegments;
#endif // MLN_TRIANGULATE_FILL_OUTLINES

    using BasicLineIndexVector = gfx::IndexVector<gfx::Lines>;
    const std::shared_ptr<BasicLineIndexVector> sharedBasicLineIndexes = std::make_shared<BasicLineIndexVector>();
    BasicLineIndexVector& basicLines = *sharedBasicLineIndexes;

    SegmentVector basicLineSegments;

    using VertexVector = gfx::VertexVector<FillLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector triangleSegments;

    std::map<std::string, FillBinders> paintPropertyBinders;
};

} // namespace mbgl
