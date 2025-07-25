#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/plugin/feature_collection.hpp>

#include <map>

/*
 
 Open Questions
 
 
 * Should we load up a vector of these RawFeatures by tile?
 * How does that interop with the render layer's update flow
 * Should the call back from render layer be more of a batch thing at that update stage?
 * Should there be a callback when a tile or collection of these features go out of scope?
 * Should the concept of managing arrays of features be something done by the core or just
   hand off the features to the plug-in layer and let it do it's thing or have the option for both?
 
 * How do we get to the osm id of features in the stream?  Is that tileFeature.getID()?
 * Is there already a set of classes or a paradigm out there that could be used to represent the
   feature / feature geometry?
 * What are the "binders"?
 
 
 Thoughts
 * Possibly have ability to keep tile coordinates using some kind flag
 
 
 
 */



namespace mbgl {

class BucketParameters;
class RenderFillLayer;

class FeatureCollectionBucket final : public Bucket {
public:
    ~FeatureCollectionBucket() override;
    //using PossiblyEvaluatedLayoutProperties = style::FillLayoutProperties::PossiblyEvaluated;

    FeatureCollectionBucket(const BucketParameters&, const std::vector<Immutable<style::LayerProperties>>&);

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

    // The tile ID
    OverscaledTileID _tileID;

    // Array of features
    std::shared_ptr<mbgl::plugin::FeatureCollection> _featureCollection = nullptr;
    //std::vector<std::shared_ptr<mbgl::plugin::Feature>> _features;
    
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
