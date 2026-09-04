#pragma once

#include <mln/tile/geometry_tile_data.hpp>
#include <mln/util/feature.hpp>

struct StubGeometryTileFeature : public mln::GeometryTileFeature {
    StubGeometryTileFeature(mln::PropertyMap properties_)
        : properties(std::move(properties_)) {}

    StubGeometryTileFeature(mln::FeatureIdentifier id_,
                            mln::FeatureType type_,
                            mln::GeometryCollection geometry_,
                            mln::PropertyMap properties_)
        : properties(std::move(properties_)),
          id(std::move(id_)),
          type(type_),
          geometry(std::move(geometry_)) {}

    mln::FeatureType getType() const override { return type; }
    mln::FeatureIdentifier getID() const override { return id; }
    const mln::GeometryCollection& getGeometries() const override { return geometry; }
    std::optional<mln::Value> getValue(const std::string& key) const override {
        const auto it = properties.find(key);
        return (it != properties.end()) ? it->second : std::optional<mln::Value>{};
    }

    mln::PropertyMap properties;
    mln::FeatureIdentifier id;
    mln::FeatureType type = mln::FeatureType::Point;
    mln::GeometryCollection geometry;
};
