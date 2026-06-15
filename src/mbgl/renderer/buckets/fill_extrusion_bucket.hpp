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
using FillExtrusionStaticVertex = gfx::Vertex<TypeList<attributes::pos>>;

#if MLN_USE_FILL_EXTRUSION_INSTANCING
using FillExtrusionLayoutVertex = gfx::Vertex<TypeList<attributes::pos, attributes::decimals_ed>>;
#else
using FillExtrusionLayoutVertex = gfx::Vertex<TypeList<attributes::pos, attributes::decimals_ed, attributes::normal2d>>;
#endif

class FillExtrusionBucket final : public Bucket {
public:
    ~FillExtrusionBucket() override;
    using PossiblyEvaluatedPaintProperties = style::FillExtrusionPaintProperties::PossiblyEvaluated;
    using PossiblyEvaluatedLayoutProperties = style::FillExtrusionLayoutProperties::PossiblyEvaluated;

    FillExtrusionBucket(const PossiblyEvaluatedLayoutProperties& layout,
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

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    static FillExtrusionLayoutVertex layoutVertex(const Point<double>& p, uint16_t edgeDistance, bool isDiscarded) {
        auto intPart = Point<double>(std::floor(p.x), std::floor(p.y));
        // Multiply factional part by 2^7 to pack them into integers [0..127]
        auto fracPart = convertPoint<uint8_t>((p - intPart) * 128.0);

        return FillExtrusionLayoutVertex{{static_cast<int16_t>(intPart.x), static_cast<int16_t>(intPart.y)},
                                         {// We pack a bool (`isDiscarded`) indicating whether this instance is discarded
                                          static_cast<uint16_t>((isDiscarded ? 0x8000 : 0) + fracPart.x * 256 + fracPart.y),
                                          // The edgeDistance attribute is used for wrapping fill_extrusion patterns
                                          edgeDistance}};
    }
#else
    static FillExtrusionLayoutVertex layoutVertex(
        const Point<double>& p, double nx, double ny, unsigned short t, uint16_t edgeDistance) {
        const auto factor = pow(2, 14);
        auto intPart = Point<double>(std::floor(p.x), std::floor(p.y));
        // Multiply factional part by 2^7 to pack them into integers [0..127]
        auto fracPart = convertPoint<uint8_t>((p - intPart) * 128.0);

        return FillExtrusionLayoutVertex{{static_cast<int16_t>(intPart.x), static_cast<int16_t>(intPart.y)},
                                         {// We pack a bool (`t`) indicating whether it is an upper or lower vertex
                                          static_cast<uint16_t>((fracPart.x * 256 + fracPart.y) * 2 + t),
                                          // The edgedistance attribute is used for wrapping fill_extrusion patterns
                                          edgeDistance},
                                          // Multiply normal vector components in the 2D plane by 2^14 to pack them into integers
                                         {static_cast<int16_t>(nx * factor), static_cast<int16_t>(ny * factor)}};
    }
#endif

    PossiblyEvaluatedLayoutProperties layout;

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
