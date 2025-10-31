#pragma once

#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/util/containers.hpp>

#include <unordered_map>
#include <functional>
#include <utility>

namespace mlt {
class MapLibreTile;
class Feature;
class Layer;
} // namespace mlt

namespace mbgl {
using MapLibreTile = mlt::MapLibreTile;

class VectorMLTTileFeature : public GeometryTileFeature {
public:
    VectorMLTTileFeature(std::shared_ptr<const MapLibreTile>,
                         const mlt::Layer& layer_,
                         const mlt::Feature&,
                         std::uint32_t extent);
    VectorMLTTileFeature(const VectorMLTTileFeature&) = delete;
    VectorMLTTileFeature(VectorMLTTileFeature&& other)
        : tile(std::move(other.tile)),
          layer(other.layer),
          feature(other.feature),
          extent(other.extent),
          version(other.version),
          lines(std::move(other.lines)),
          properties(std::move(other.properties)) {}

    VectorMLTTileFeature& operator=(VectorMLTTileFeature&&) = delete;
    VectorMLTTileFeature& operator=(const VectorMLTTileFeature&) = delete;

    FeatureType getType() const override;
    std::optional<Value> getValue(const std::string& key) const override;
    const PropertyMap& getProperties() const override;
    FeatureIdentifier getID() const override;
    const GeometryCollection& getGeometries() const override;

private:
    std::shared_ptr<const MapLibreTile> tile;
    const mlt::Layer& layer;
    mlt::Feature const& feature;
    std::uint32_t extent;
    int version;

    // lazy init
    mutable std::optional<GeometryCollection> lines;
    mutable std::optional<PropertyMap> properties;
};

class VectorMLTTileLayer : public GeometryTileLayer {
public:
    VectorMLTTileLayer(std::shared_ptr<const MapLibreTile>, const mlt::Layer&);

    std::size_t featureCount() const override;
    std::unique_ptr<GeometryTileFeature> getFeature(std::size_t i) const override;
    std::string getName() const override;

private:
    const std::shared_ptr<const MapLibreTile> tile;
    const mlt::Layer& layer;
};

class VectorMLTTileData : public GeometryTileData {
public:
    VectorMLTTileData(std::shared_ptr<const std::string> data);
    VectorMLTTileData(const VectorMLTTileData&);
    VectorMLTTileData(VectorMLTTileData&&) = default;

    std::unique_ptr<GeometryTileData> clone() const override;
    std::unique_ptr<GeometryTileLayer> getLayer(const std::string& name) const override;

    std::vector<std::string> layerNames() const;

private:
    mutable std::shared_ptr<const std::string> data;
    mutable std::shared_ptr<const MapLibreTile> tile;
};

} // namespace mbgl
