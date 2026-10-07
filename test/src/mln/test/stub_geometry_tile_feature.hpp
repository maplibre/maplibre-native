#include <mln/tile/geometry_tile_data.hpp>
#include <mln/util/feature.hpp>

namespace mln {

struct StubGeometryTileFeature : public GeometryTileFeature {
    StubGeometryTileFeature(PropertyMap properties_)
        : properties(std::move(properties_)) {}

    StubGeometryTileFeature(FeatureIdentifier id_,
                            FeatureType type_,
                            GeometryCollection geometry_,
                            PropertyMap properties_)
        : properties(std::move(properties_)),
          id(std::move(id_)),
          type(type_),
          geometry(std::move(geometry_)) {}

    StubGeometryTileFeature(FeatureType type_, GeometryCollection geometry_)
        : type(type_),
          geometry(std::move(geometry_)) {}

    FeatureType getType() const override { return type; }
    FeatureIdentifier getID() const override { return id; }
    const GeometryCollection& getGeometries() const override { return geometry; }
    std::optional<Value> getValue(const std::string& key) const override {
        const auto it = properties.find(key);
        return (it != properties.end()) ? it->second : std::optional<Value>();
    }

    PropertyMap properties;
    FeatureIdentifier id;
    FeatureType type = FeatureType::Point;
    GeometryCollection geometry;
};

} // namespace mln
