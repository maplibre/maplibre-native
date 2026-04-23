#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/renderer/render_light.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_properties.hpp>

namespace mbgl {

class BucketParameters;
class RenderFillExtrusionLayer;

using FillExtrusionBinders = PaintPropertyBinders<style::FillExtrusionPaintProperties::DataDrivenProperties>;
using FillExtrusionLayoutVertex = gfx::Vertex<TypeList<attributes::pos, attributes::ed_discard>>;
using FillExtrusionStaticVertex = gfx::Vertex<TypeList<attributes::pos>>;

class FillExtrusionBucket final : public Bucket {
public:
    ~FillExtrusionBucket() override;
    using PossiblyEvaluatedPaintProperties = style::FillExtrusionPaintProperties::PossiblyEvaluated;
    using PossiblyEvaluatedLayoutProperties = style::Properties<>::PossiblyEvaluated;

    FillExtrusionBucket(const PossiblyEvaluatedLayoutProperties&,
                        const std::map<std::string, Immutable<style::LayerProperties>>&,
                        float,
                        uint32_t);

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

    static FillExtrusionLayoutVertex layoutVertex(Point<int16_t> p, uint16_t edgeDistance, bool isDiscarded) {
        return FillExtrusionLayoutVertex{{p.x, p.y},
                                         {// The edgeDistance attribute is used for wrapping fill_extrusion patterns
                                          edgeDistance,
                                          // When used as instance vector this specify if an instance is discarded
                                          static_cast<uint16_t>(isDiscarded)}};
    }

    static std::array<float, 3> lightColor(const EvaluatedLight&);
    static std::array<float, 3> lightPosition(const EvaluatedLight&, const TransformState&);
    static float lightIntensity(const EvaluatedLight&);

    using VertexVector = gfx::VertexVector<FillExtrusionLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector triangleSegments;

    std::unordered_map<std::string, FillExtrusionBinders> paintPropertyBinders;
};

} // namespace mbgl
