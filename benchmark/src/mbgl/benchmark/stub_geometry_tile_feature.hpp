#pragma once

#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/util/feature.hpp>

struct StubGeometryTileFeature : public mbgl::GeometryTileFeature {
    StubGeometryTileFeature(mbgl::PropertyMap properties_)
        : properties(std::move(properties_)) {}

    StubGeometryTileFeature(mbgl::FeatureIdentifier id_,
                            mbgl::FeatureType type_,
                            mbgl::GeometryCollection geometry_,
                            mbgl::PropertyMap properties_)
        : properties(std::move(properties_)),
          id(std::move(id_)),
          type(type_),
          geometry(std::move(geometry_)) {}

    mbgl::FeatureType getType() const override { return type; }
    mbgl::FeatureIdentifier getID() const override { return id; }
    const mbgl::GeometryCollection& getGeometries() const override { return geometry; }
    std::optional<mbgl::Value> getValue(const std::string& key) const override {
        const auto it = properties.find(key);
        return (it != properties.end()) ? it->second : std::optional<mbgl::Value>{};
    }

    mbgl::PropertyMap properties;
    mbgl::FeatureIdentifier id;
    mbgl::FeatureType type = mbgl::FeatureType::Point;
    mbgl::GeometryCollection geometry;
};
