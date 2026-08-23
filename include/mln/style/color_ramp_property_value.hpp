#pragma once

#include <mln/util/variant.hpp>
#include <mln/style/undefined.hpp>
#include <mln/style/expression/expression.hpp>

namespace mln {
namespace style {

/**
 * Special-case implementation of (a subset of) the PropertyValue<T> interface
 * used for building the HeatmapColor paint property traits class.
 */
class ColorRampPropertyValue {
private:
    std::shared_ptr<expression::Expression> value;
    std::shared_ptr<const std::set<std::string>> globalStateRefs_;

    friend bool operator==(const ColorRampPropertyValue& lhs, const ColorRampPropertyValue& rhs) noexcept {
        return (lhs.isUndefined() && rhs.isUndefined()) || (lhs.value && rhs.value && *(lhs.value) == *(rhs.value));
    }

    friend bool operator!=(const ColorRampPropertyValue& lhs, const ColorRampPropertyValue& rhs) noexcept {
        return !(lhs == rhs);
    }

public:
    ColorRampPropertyValue() noexcept = default;
    ColorRampPropertyValue(std::shared_ptr<expression::Expression> value_)
        : value(std::move(value_)) {
        if (value && value->has(expression::Dependency::GlobalState)) {
            auto refs = std::make_shared<std::set<std::string>>();
            expression::collectGlobalStateRefs(*value, *refs);
            globalStateRefs_ = std::move(refs);
        }
    }

    bool isUndefined() const noexcept { return value == nullptr; }

    // noop, needed for batch evaluation of paint property values to compile
    template <typename Evaluator>
    Color evaluate(const Evaluator&, TimePoint = {}) const noexcept {
        return {};
    }

    Color evaluate(double rampEvaluationParameter, const GlobalStateMap* globalState = nullptr) const {
        const auto result = value->evaluate(
            expression::EvaluationContext({}, nullptr, {rampEvaluationParameter}).withGlobalState(globalState));
        if (!result) {
            return {};
        }
        const auto color = expression::fromExpressionValue<Color>(*result);
        return color ? *color : Color{};
    }

    bool isDataDriven() const noexcept { return false; }
    bool hasDataDrivenPropertyDifference(const ColorRampPropertyValue&) const noexcept { return false; }

    const expression::Expression& getExpression() const noexcept { return *value; }

    using Dependency = style::expression::Dependency;
    Dependency getDependencies() const noexcept { return value ? value->dependencies : Dependency::None; }

    const std::set<std::string>* getGlobalStateRefs() const noexcept { return globalStateRefs_.get(); }
};

} // namespace style
} // namespace mln
