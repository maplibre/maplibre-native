#pragma once

#include <mbgl/util/variant.hpp>
#include <mbgl/style/undefined.hpp>
#include <mbgl/style/property_expression.hpp>

namespace mbgl {
namespace style {

template <class T>
class PropertyValue {
private:
    using Value = std::variant<Undefined, T, PropertyExpression<T>>;

    Value value;

    friend bool operator==(const PropertyValue& lhs, const PropertyValue& rhs) noexcept {
        return lhs.value == rhs.value;
    }

    friend bool operator!=(const PropertyValue& lhs, const PropertyValue& rhs) noexcept { return !(lhs == rhs); }

public:
    PropertyValue() = default;

    PropertyValue(T constant)
        : value(std::forward<T>(constant)) {}

    PropertyValue(PropertyExpression<T> expression) noexcept
        : value(std::move(expression)) {}

    bool isUndefined() const noexcept { return std::holds_alternative<Undefined>(value); }

    bool isConstant() const noexcept { return std::holds_alternative<T>(value); }

    bool isExpression() const noexcept { return std::holds_alternative<PropertyExpression<T>>(value); }

    bool isDataDriven() const noexcept {
        return std::visit(overloaded{[](const Undefined&) { return false; },
                                     [](const T&) { return false; },
                                     [](const PropertyExpression<T>& fn) {
                                         return !fn.isFeatureConstant();
                                     }},
                          value);
    }

    bool isZoomConstant() const noexcept {
        return std::visit(overloaded{[](const Undefined&) { return true; },
                                     [](const T&) { return true; },
                                     [](const PropertyExpression<T>& fn) {
                                         return fn.isZoomConstant();
                                     }},
                          value);
    }

    const T& asConstant() const noexcept { return std::get<T>(value); }

    PropertyExpression<T>& asExpression() noexcept { return std::get<PropertyExpression<T>>(value); }
    const PropertyExpression<T>& asExpression() const noexcept { return std::get<PropertyExpression<T>>(value); }

    template <class... Ts>
    auto match(Ts&&... ts) const {
        return std::visit(overloaded{std::forward<Ts>(ts)...}, value);
    }

    template <typename Evaluator>
    auto evaluate(const Evaluator& evaluator, TimePoint = {}) const {
        return std::visit(evaluator, value);
    }

    bool hasDataDrivenPropertyDifference(const PropertyValue<T>& other) const noexcept {
        return *this != other && (isDataDriven() || other.isDataDriven());
    }

    using Dependency = style::expression::Dependency;
    Dependency getDependencies() const noexcept {
        return std::visit(overloaded{[](const Undefined&) { return Dependency::None; },
                                     [](const T&) { return Dependency::None; },
                                     [](const PropertyExpression<T>& ex) {
                                         return ex.getDependencies();
                                     }},
                          value);
    }
};

} // namespace style
} // namespace mbgl
