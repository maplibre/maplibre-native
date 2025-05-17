#pragma once
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>
#include <mbgl/style/image_impl.hpp>

namespace mbgl {

class BucketParameters;
class RenderLineLayer;

using LineBinders = PaintPropertyBinders<style::LinePaintProperties::DataDrivenProperties>;
using LineLayoutVertex = gfx::Vertex<TypeList<attributes::pos_normal, attributes::data<uint8_t, 4>>>;

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

    /*
     * @param p vertex position
     * @param e extrude normal
     * @param round whether the vertex uses a round line cap
     * @param up whether the line normal points up or down
     * @param dir direction of the line cap (-1/0/1)
     */
    static LineLayoutVertex layoutVertex(
        Point<int16_t> p, Point<double> e, bool round, bool up, int8_t dir, int32_t linesofar = 0) {
        return LineLayoutVertex{
            {{static_cast<int16_t>((p.x * 2) | (round ? 1 : 0)), static_cast<int16_t>((p.y * 2) | (up ? 1 : 0))}},
            {{// add 128 to store a byte in an unsigned byte
              static_cast<uint8_t>(util::clamp(::round(extrudeScale * e.x) + 128, 0., 255.)),
              static_cast<uint8_t>(util::clamp(::round(extrudeScale * e.y) + 128, 0., 255.)),

              // Encode the -1/0/1 direction value into the first two bits of .z
              // of a_data. Combine it with the lower 6 bits of `linesofar`
              // (shifted by 2 bites to make room for the direction value). The
              // upper 8 bits of `linesofar` are placed in the `w` component.
              // `linesofar` is scaled down by `LINE_DISTANCE_SCALE` so that we
              // can store longer distances while sacrificing precision.

              // Encode the -1/0/1 direction value into .zw coordinates of
              // a_data, which is normally covered by linesofar, so we need to
              // merge them. The z component's first bit, as well as the sign
              // bit is reserved for the direction, so we need to shift the
              // linesofar.
              static_cast<uint8_t>(((dir == 0 ? 0 : (dir < 0 ? -1 : 1)) + 1) | ((linesofar & 0x3F) << 2)),
              static_cast<uint8_t>(linesofar >> 6)}}};
    }

    /*
     * Scale the extrusion vector so that the normal length is this value.
     * Contains the "texture" normals (-1..1). This is distinct from the extrude
     * normals for line joins, because the x-value remains 0 for the texture
     * normal array, while the extrude normal actually moves the vertex to
     * create the acute/bevelled line join.
     */
    static const int8_t extrudeScale = 63;

    PossiblyEvaluatedLayoutProperties layout;

    using VertexVector = gfx::VertexVector<LineLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector segments;

    std::map<std::string, LineBinders> paintPropertyBinders;

private:
    void addGeometry(const GeometryCoordinates&, const GeometryTileFeature&, const CanonicalTileID&);

    const float zoom;
    const uint32_t overscaling;
};

} // namespace mbgl
