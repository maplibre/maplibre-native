#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/util/geojson.hpp>

#include <optional>

namespace mln {
namespace style {
namespace expression {

class Within final : public Expression {
public:
    explicit Within(GeoJSON geojson, Feature::geometry_type geometries_);

    ~Within() override;

    EvaluationResult evaluate(const EvaluationContext&) const override;

    static ParseResult parse(const mln::style::conversion::Convertible&, ParsingContext&);

    void eachChild(const std::function<void(const Expression&)>&) const noexcept override {}

    bool operator==(const Expression& e) const noexcept override;

    std::vector<std::optional<Value>> possibleOutputs() const override;

    mln::Value serialize() const override;
    std::string getOperator() const override { return "within"; }

private:
    GeoJSON geoJSONSource;
    Feature::geometry_type geometries;
};

} // namespace expression
} // namespace style
} // namespace mln
