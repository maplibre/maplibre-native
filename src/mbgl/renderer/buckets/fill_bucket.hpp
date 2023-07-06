#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/programs/fill_program.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>

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

    using VertexVector = gfx::VertexVector<FillLayoutVertex>;
    std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    gfx::IndexVector<gfx::Lines> lines;
    gfx::IndexVector<gfx::Triangles> triangles;
    SegmentVector<FillAttributes> lineSegments;
    SegmentVector<FillAttributes> triangleSegments;

#if MLN_LEGACY_RENDERER
    std::optional<gfx::VertexBuffer<FillLayoutVertex>> vertexBuffer;
    std::optional<gfx::IndexBuffer> lineIndexBuffer;
    std::optional<gfx::IndexBuffer> triangleIndexBuffer;
#endif // MLN_LEGACY_RENDERER

    std::map<std::string, FillProgram::Binders> paintPropertyBinders;
};

} // namespace mbgl
