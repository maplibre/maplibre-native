#pragma once

#include <mbgl/util/variant.hpp>
#include <mbgl/style/undefined.hpp>
#include <mbgl/style/property_expression.hpp>

namespace mbgl {
namespace style {

template <class T>
class PropertyValue {
private:
    using Value = variant<Undefined, T, PropertyExpression<T>>;

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

    bool isUndefined() const noexcept { return value.template is<Undefined>(); }

    bool isConstant() const noexcept { return value.template is<T>(); }

    bool isExpression() const noexcept { return value.template is<PropertyExpression<T>>(); }

    bool isDataDriven() const noexcept {
        return value.match([](const Undefined&) { return false; },
                           [](const T&) { return false; },
                           [](const PropertyExpression<T>& fn) { return !fn.isFeatureConstant(); });
    }

    bool isZoomConstant() const noexcept {
        return value.match([](const Undefined&) { return true; },
                           [](const T&) { return true; },
                           [](const PropertyExpression<T>& fn) { return fn.isZoomConstant(); });
    }

    const T& asConstant() const noexcept { return value.template get<T>(); }

    PropertyExpression<T>& asExpression() noexcept { return value.template get<PropertyExpression<T>>(); }
    const PropertyExpression<T>& asExpression() const noexcept { return value.template get<PropertyExpression<T>>(); }

    template <class... Ts>
    auto match(Ts&&... ts) const {
        return value.match(std::forward<Ts>(ts)...);
    }

    template <typename Evaluator>
    auto evaluate(const Evaluator& evaluator, TimePoint = {}) const {
        return Value::visit(value, evaluator);
    }

    bool hasDataDrivenPropertyDifference(const PropertyValue<T>& other) const noexcept {
        return *this != other && (isDataDriven() || other.isDataDriven());
    }

    using Dependency = style::expression::Dependency;
    Dependency getDependencies() const noexcept {
        return value.match([](const Undefined&) { return Dependency::None; },
                           [](const T&) { return Dependency::None; },
                           [](const PropertyExpression<T>& ex) { return ex.getDependencies(); });
    }
};

} // namespace style
} // namespace mbgl
