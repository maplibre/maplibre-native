#pragma once

#include <mbgl/util/variant.hpp>
#include <mbgl/style/undefined.hpp>
#include <mbgl/style/expression/expression.hpp>

namespace mbgl {
namespace style {

/**
 * Special-case implementation of (a subset of) the PropertyValue<T> interface
 * used for building the HeatmapColor paint property traits class.
 */
class ColorRampPropertyValue {
private:
    std::shared_ptr<expression::Expression> value;

    friend bool operator==(const ColorRampPropertyValue& lhs, const ColorRampPropertyValue& rhs) noexcept {
        return (lhs.isUndefined() && rhs.isUndefined()) || (lhs.value && rhs.value && *(lhs.value) == *(rhs.value));
    }

    friend bool operator!=(const ColorRampPropertyValue& lhs, const ColorRampPropertyValue& rhs) noexcept {
        return !(lhs == rhs);
    }

public:
    ColorRampPropertyValue() noexcept = default;
    ColorRampPropertyValue(std::shared_ptr<expression::Expression> value_) noexcept
        : value(std::move(value_)) {}

    bool isUndefined() const noexcept { return value == nullptr; }

    // noop, needed for batch evaluation of paint property values to compile
    template <typename Evaluator>
    Color evaluate(const Evaluator&, TimePoint = {}) const noexcept {
        return {};
    }

    Color evaluate(double rampEvaluationParameter) const {
        const auto result = value->evaluate(expression::EvaluationContext({}, nullptr, {rampEvaluationParameter}));
        return *expression::fromExpressionValue<Color>(*result);
    }

    bool isDataDriven() const noexcept { return false; }
    bool hasDataDrivenPropertyDifference(const ColorRampPropertyValue&) const noexcept { return false; }

    const expression::Expression& getExpression() const noexcept { return *value; }

    using Dependency = style::expression::Dependency;
    Dependency getDependencies() const noexcept { return value ? value->dependencies : Dependency::None; }
};

} // namespace style
} // namespace mbgl
