#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/programs/circle_program.hpp>
#include <mbgl/style/layers/circle_layer_properties.hpp>

namespace mbgl {

class BucketParameters;

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

    using VertexVector = gfx::VertexVector<CircleLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector segments;

    std::map<std::string, CircleProgram::Binders> paintPropertyBinders;

    const MapMode mode;
};

} // namespace mbgl
