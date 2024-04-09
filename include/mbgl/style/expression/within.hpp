#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/util/geojson.hpp>

#include <optional>

namespace mbgl {
namespace style {
namespace expression {

class Within final : public Expression {
public:
    explicit Within(GeoJSON geojson, Feature::geometry_type geometries_);

    ~Within() override;

    EvaluationResult evaluate(const EvaluationContext&) const override;

    static ParseResult parse(const mbgl::style::conversion::Convertible&, ParsingContext&);

    void eachChild(const std::function<void(const Expression&)>&) const noexcept override {}

    bool operator==(const Expression& e) const noexcept override;

    std::vector<std::optional<Value>> possibleOutputs() const override;

    mbgl::Value serialize() const override;
    std::string getOperator() const override { return "within"; }

private:
    GeoJSON geoJSONSource;
    Feature::geometry_type geometries;
};

} // namespace expression
} // namespace style
} // namespace mbgl
