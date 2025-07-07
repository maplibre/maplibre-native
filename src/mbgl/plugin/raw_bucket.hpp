#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>

#include <map>

namespace mbgl {

class BucketParameters;
class RenderFillLayer;

//using FillBinders = PaintPropertyBinders<style::FillPaintProperties::DataDrivenProperties>;
//using FillLayoutVertex = gfx::Vertex<TypeList<attributes::pos>>;

class RawBucketFeatureCoordinate {
public:
    RawBucketFeatureCoordinate(double lat, double lon) : _lat(lat), _lon(lon) {
        
    }
    double _lat = 0;
    double _lon = 0;
};

// This is a list of coordinates.  Broken out into it's own class because
// a raw bucket feature can have an array of these
class RawBucketFeatureCoordinateCollection {
public:
    std::vector<RawBucketFeatureCoordinate> _coordinates;
};

class RawBucketFeature {
public:
    RawBucketFeature() {};
    enum class FeatureType {
        FeatureTypeUnknown,
        FeatureTypePoint,
        FeatureTypeLine,
        FeatureTypePolygon
    };
    FeatureType _featureType = FeatureType::FeatureTypeUnknown;
    std::map<std::string, std::string> _featureProperties;
    std::vector<RawBucketFeatureCoordinateCollection> _featureCoordinates;
};

class RawBucket final : public Bucket {
public:
    ~RawBucket() override;
    //using PossiblyEvaluatedLayoutProperties = style::FillLayoutProperties::PossiblyEvaluated;

    RawBucket(const BucketParameters&, const std::vector<Immutable<style::LayerProperties>>&);

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

    // Array of features
    std::vector<std::shared_ptr<RawBucketFeature>> _features;
    
    std::vector<Immutable<style::LayerProperties>> _layers;
    
    /*
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
     */
};

} // namespace mbgl
