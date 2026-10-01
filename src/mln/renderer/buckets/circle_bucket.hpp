#pragma once

#include <mln/renderer/bucket.hpp>
#include <mln/renderer/paint_property_binder.hpp>
#include <mln/map/mode.hpp>
#include <mln/tile/geometry_tile_data.hpp>
#include <mln/gfx/vertex_buffer.hpp>
#include <mln/gfx/index_buffer.hpp>
#include <mln/shaders/segment.hpp>
#include <mln/style/layers/circle_layer_properties.hpp>

namespace mln {

class BucketParameters;

using CircleBinders = PaintPropertyBinders<style::CirclePaintProperties::DataDrivenProperties>;
using CircleLayoutVertex = gfx::Vertex<TypeList<attributes::pos>>;

class CircleBucket final : public Bucket {
public:
    using PossiblyEvaluatedLayoutProperties = style::CircleLayoutProperties::PossiblyEvaluated;

    CircleBucket(const std::map<std::string, Immutable<style::LayerProperties>>& layerPaintProperties,
                 MapMode mode,
                 float zoom);
    ~CircleBucket() override;

    bool hasData() const override;

    void upload(gfx::UploadPass&) override;

    float getQueryRadius(const RenderLayer&) const override;

    void update(const FeatureStates&, const GeometryTileLayer&, const std::string&, const ImagePositions&) override;

    /*
     * @param {number} x vertex position
     * @param {number} y vertex position
     * @param {number} ex extrude normal
     * @param {number} ey extrude normal
     */
    static CircleLayoutVertex vertex(Point<int16_t> p, float ex, float ey) {
        return CircleLayoutVertex{
            {{static_cast<int16_t>((p.x * 2) + ((ex + 1) / 2)), static_cast<int16_t>((p.y * 2) + ((ey + 1) / 2))}}};
    }

    using VertexVector = gfx::VertexVector<CircleLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector segments;

    std::map<std::string, CircleBinders> paintPropertyBinders;

    const MapMode mode;
};

} // namespace mln
