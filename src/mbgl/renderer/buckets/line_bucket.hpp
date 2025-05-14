#pragma once
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/programs/line_program.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>
#include <mbgl/style/image_impl.hpp>

namespace mbgl {

class BucketParameters;
class RenderLineLayer;

class LineBucket final : public Bucket {
public:
    using PossiblyEvaluatedLayoutProperties = style::LineLayoutProperties::PossiblyEvaluated;

    LineBucket(PossiblyEvaluatedLayoutProperties layout,
               const std::map<std::string, Immutable<style::LayerProperties>>& layerPaintProperties,
               float zoom,
               uint32_t overscaling);
    ~LineBucket() override;

    void addFeature(const GeometryTileFeature&,
                    const GeometryCollection&,
                    const mbgl::ImagePositions& patternPositions,
                    const PatternLayerMap&,
                    std::size_t,
                    const CanonicalTileID&) override;

    bool hasData() const override;

    void upload(gfx::UploadPass&) override;

    float getQueryRadius(const RenderLayer&) const override;

    void update(const FeatureStates&, const GeometryTileLayer&, const std::string&, const ImagePositions&) override;

    PossiblyEvaluatedLayoutProperties layout;

    using VertexVector = gfx::VertexVector<LineLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector segments;

    std::map<std::string, LineProgram::Binders> paintPropertyBinders;

private:
    void addGeometry(const GeometryCoordinates&, const GeometryTileFeature&, const CanonicalTileID&);

    const float zoom;
    const uint32_t overscaling;
};

} // namespace mbgl
