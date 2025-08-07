#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/plugin/feature_collection.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>

#include <map>

namespace mbgl {

class BucketParameters;
class RenderFillLayer;

class FeatureCollectionBucket final : public Bucket {
public:
    ~FeatureCollectionBucket() override;

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

    // Feature collection is an list of features
    std::shared_ptr<mbgl::plugin::FeatureCollection> _featureCollection = nullptr;
    std::vector<Immutable<style::LayerProperties>> _layers;
};

} // namespace mbgl
