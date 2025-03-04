#include <mbgl/style/expression/compound_expression.hpp>
#include <mbgl/style/expression/expression.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>

#include <sstream>
#include <utility>

namespace mbgl {

using Dependency = style::expression::Dependency;

std::ostream& operator<<(std::ostream& os, const Dependency deps) {
    if (deps == Dependency::None) {
        os << "None";
        return os;
    }

    bool any = false;
    const auto add = [&](Dependency dep, std::string_view name) {
        if (deps & dep) {
            os << (any ? "|" : std::string_view{}) << name;
            any = true;
        }
    };

    add(Dependency::Feature, "Feature");
    add(Dependency::Image, "Image");
    add(Dependency::Zoom, "Zoom");
    add(Dependency::Location, "Location");
    add(Dependency::Bind, "Bind");
    add(Dependency::Var, "Var");
    add(Dependency::Override, "Override");

    return os;
}

namespace util {

std::string toString(const style::expression::Dependency deps) {
    std::ostringstream ss;
    ss << deps;
    return ss.str();
}

} // namespace util

namespace style {
namespace expression {

class GeoJSONFeature : public GeometryTileFeature {
public:
    const Feature& feature;
    mutable std::optional<GeometryCollection> geometry;

    explicit GeoJSONFeature(const Feature& feature_)
        : feature(feature_) {}
    GeoJSONFeature(const Feature& feature_, const CanonicalTileID& canonical)
        : feature(feature_) {
        geometry = convertGeometry(feature.geometry, canonical);
        // https://github.com/mapbox/geojson-vt-cpp/issues/44
        if (getTypeImpl() == FeatureType::Polygon) {
            geometry = fixupPolygons(*geometry);
        }
    }

    FeatureType getType() const override { return getTypeImpl(); }

    const PropertyMap& getProperties() const override { return feature.properties; }
    FeatureIdentifier getID() const override { return feature.id; }
    std::optional<mbgl::Value> getValue(const std::string& key) const override {
        auto it = feature.properties.find(key);
        if (it != feature.properties.end()) {
            return std::optional<mbgl::Value>(it->second);
        }
        return std::optional<mbgl::Value>();
    }
    const GeometryCollection& getGeometries() const override {
        if (geometry) return *geometry;
        geometry = GeometryCollection();
        return *geometry;
    }

private:
    FeatureType getTypeImpl() const { return apply_visitor(ToFeatureType(), feature.geometry); }
};

EvaluationResult Expression::evaluate(std::optional<float> zoom,
                                      const Feature& feature,
                                      std::optional<double> colorRampParameter) const {
    GeoJSONFeature f(feature);
    return this->evaluate(EvaluationContext(std::move(zoom), &f, std::move(colorRampParameter)));
}

EvaluationResult Expression::evaluate(std::optional<float> zoom,
                                      const Feature& feature,
                                      std::optional<double> colorRampParameter,
                                      const std::set<std::string>& availableImages) const {
    GeoJSONFeature f(feature);
    return this->evaluate(
        EvaluationContext(std::move(zoom), &f, std::move(colorRampParameter)).withAvailableImages(&availableImages));
}

EvaluationResult Expression::evaluate(std::optional<float> zoom,
                                      const Feature& feature,
                                      std::optional<double> colorRampParameter,
                                      const std::set<std::string>& availableImages,
                                      const CanonicalTileID& canonical) const {
    GeoJSONFeature f(feature, canonical);
    return this->evaluate(EvaluationContext(std::move(zoom), &f, std::move(colorRampParameter))
                              .withAvailableImages(&availableImages)
                              .withCanonicalTileID(&canonical));
}

EvaluationResult Expression::evaluate(std::optional<mbgl::Value> accumulated, const Feature& feature) const {
    GeoJSONFeature f(feature);
    return this->evaluate(EvaluationContext(std::move(accumulated), &f));
}

} // namespace expression
} // namespace style
} // namespace mbgl
