#pragma once

#include <mbgl/style/expression/image.hpp>
#include <mbgl/text/tagged_string.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>

#include <array>
#include <string>
#include <optional>

namespace mbgl {

class SymbolFeature : public GeometryTileFeature {
public:
    SymbolFeature(std::unique_ptr<GeometryTileFeature> feature_)
        : feature(std::move(feature_)),
          geometry(feature->getGeometries().clone()) // we need a mutable copy of the geometry for mergeLines()
    {}

    FeatureType getType() const override { return feature->getType(); }
    std::optional<Value> getValue(const std::string& key) const override { return feature->getValue(key); }
    const PropertyMap& getProperties() const override { return feature->getProperties(); }
    FeatureIdentifier getID() const override { return feature->getID(); };
    const GeometryCollection& getGeometries() const override { return feature->getGeometries(); }

    friend bool operator<(const SymbolFeature& lhs, const SymbolFeature& rhs) { return lhs.sortKey < rhs.sortKey; }

    std::unique_ptr<GeometryTileFeature> feature;
    GeometryCollection geometry;
    std::optional<TaggedString> formattedText;
    std::optional<style::expression::Image> icon;
    float sortKey = 0.0f;
    std::size_t index = 0;
    bool allowsVerticalWritingMode = false;
};

} // namespace mbgl
