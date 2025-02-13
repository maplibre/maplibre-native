#pragma once

#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/util/containers.hpp>

#include <protozero/pbf_reader.hpp>

#include <unordered_map>
#include <functional>
#include <utility>

namespace mbgl {

class VectorMLTTileFeature : public GeometryTileFeature {
public:
    VectorMLTTileFeature(/*const mapbox::vector_tile::layer&, const protozero::data_view&*/);

    FeatureType getType() const override;
    std::optional<Value> getValue(const std::string& key) const override;
    const PropertyMap& getProperties() const override;
    FeatureIdentifier getID() const override;
    const GeometryCollection& getGeometries() const override;

private:
    //mapbox::vector_tile::feature feature;
    mutable std::optional<GeometryCollection> lines;
    mutable std::optional<PropertyMap> properties;
};

class VectorMLTTileLayer : public GeometryTileLayer {
public:
    VectorMLTTileLayer(std::shared_ptr<const std::string> data, const protozero::data_view&);

    std::size_t featureCount() const override;
    std::unique_ptr<GeometryTileFeature> getFeature(std::size_t i) const override;
    std::string getName() const override;

private:
    std::shared_ptr<const std::string> data;
    //mapbox::vector_tile::layer layer;
};

class VectorMLTTileData : public GeometryTileData {
public:
    VectorMLTTileData(std::shared_ptr<const std::string> data);

    std::unique_ptr<GeometryTileData> clone() const override;
    std::unique_ptr<GeometryTileLayer> getLayer(const std::string& name) const override;

    std::vector<std::string> layerNames() const;

private:
    std::shared_ptr<const std::string> data;
    mutable bool parsed = false;
    mutable mbgl::unordered_map<std::string, const protozero::data_view> layers;
};

} // namespace mbgl
