#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/style/layers/heatmap_layer_properties.hpp>

namespace mbgl {

class BucketParameters;

using HeatmapBinders = PaintPropertyBinders<style::HeatmapPaintProperties::DataDrivenProperties>;
using HeatmapLayoutVertex = gfx::Vertex<TypeList<attributes::pos>>;
using HeatmapTextureLayoutVertex = gfx::Vertex<TypeList<attributes::pos>>;

class HeatmapBucket final : public Bucket {
public:
    HeatmapBucket(const BucketParameters&, const std::vector<Immutable<style::LayerProperties>>&);
    ~HeatmapBucket() override;

    void addFeature(const GeometryTileFeature&,
                    const GeometryCollection&,
                    const ImagePositions&,
                    const PatternLayerMap&,
                    std::size_t,
                    const CanonicalTileID&) override;
    bool hasData() const override;

    void upload(gfx::UploadPass&) override;

    float getQueryRadius(const RenderLayer&) const override;

    /*
     * @param {number} x vertex position
     * @param {number} y vertex position
     * @param {number} ex extrude normal
     * @param {number} ey extrude normal
     */
    static HeatmapLayoutVertex vertex(Point<int16_t> p, float ex, float ey) {
        return HeatmapLayoutVertex{
            {{static_cast<int16_t>((p.x * 2) + ((ex + 1) / 2)), static_cast<int16_t>((p.y * 2) + ((ey + 1) / 2))}}};
    }

    static HeatmapTextureLayoutVertex textureVertex(Point<int16_t> p) {
        return HeatmapTextureLayoutVertex{{{p.x, p.y}}};
    }

    using VertexVector = gfx::VertexVector<HeatmapLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector segments;

    std::map<std::string, HeatmapBinders> paintPropertyBinders;

    const MapMode mode;
};

} // namespace mbgl
