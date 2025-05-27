#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/programs/heatmap_program.hpp>
#include <mbgl/style/layers/heatmap_layer_properties.hpp>

namespace mbgl {

class BucketParameters;

/*
 TODO: This is not used, but I kind of like the idea of leaving it here as a placeholder.
 What is the MVP of that?
 */

class PluginLayerBucket final : public Bucket {
public:
    PluginLayerBucket(const BucketParameters&, const std::vector<Immutable<style::LayerProperties>>&);
    ~PluginLayerBucket() override;

    void addFeature(const GeometryTileFeature&,
                    const GeometryCollection&,
                    const ImagePositions&,
                    const PatternLayerMap&,
                    std::size_t,
                    const CanonicalTileID&) override;
    bool hasData() const override;

    void upload(gfx::UploadPass&) override;

    float getQueryRadius(const RenderLayer&) const override;

    using VertexVector = gfx::VertexVector<HeatmapLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    // SegmentVector<HeatmapAttributes> segments;

    // std::map<std::string, HeatmapProgram::Binders> paintPropertyBinders;

    const MapMode mode;
};

} // namespace mbgl
