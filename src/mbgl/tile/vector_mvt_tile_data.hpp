#pragma once
#include <mbgl/tile/geometry_tile_data.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244 4267)
#endif

#include <mapbox/vector_tile.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <protozero/pbf_reader.hpp>

#include <unordered_map>
#include <functional>
#include <utility>

namespace mbgl {

class VectorMVTTileFeature : public GeometryTileFeature {
public:
    VectorMVTTileFeature(const mapbox::vector_tile::layer&, const protozero::data_view&);

    FeatureType getType() const override;
    std::optional<Value> getValue(const std::string& key) const override;
    const PropertyMap& getProperties() const override;
    FeatureIdentifier getID() const override;
    const GeometryCollection& getGeometries() const override;

private:
    mapbox::vector_tile::feature feature;
    mutable std::optional<GeometryCollection> lines;
    mutable std::optional<PropertyMap> properties;
};

class VectorMVTTileLayer : public GeometryTileLayer {
public:
    VectorMVTTileLayer(std::shared_ptr<const std::string> data, const protozero::data_view&);

    std::size_t featureCount() const override;
    std::unique_ptr<GeometryTileFeature> getFeature(std::size_t i) const override;
    std::string getName() const override;

private:
    std::shared_ptr<const std::string> data;
    mapbox::vector_tile::layer layer;
};

class VectorMVTTileData : public GeometryTileData {
public:
    VectorMVTTileData(std::shared_ptr<const std::string> data);

    std::unique_ptr<GeometryTileData> clone() const override;
    std::unique_ptr<GeometryTileLayer> getLayer(const std::string& name) const override;

    std::vector<std::string> layerNames() const;

private:
    std::shared_ptr<const std::string> data;
    mutable bool parsed = false;
    mutable std::map<std::string, const protozero::data_view> layers;
};

} // namespace mbgl
