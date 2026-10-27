#include <mln/style/plugin_property.hpp>
#include <mln/renderer/data_driven_property_evaluator.hpp>

#include <mln/plugin/plugin_registry.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/stringify.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/expression/expression.hpp>
#include <mln/tile/geometry_tile_data.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/interpolate.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace mln::util {
template <>
struct Interpolator<style::expression::Image> : Uninterpolated {};
} // namespace mln::util

namespace mln {
namespace style {
namespace {

bool containsOperator(const mln::Value& value, const std::string& name) {
    const auto* array = value.getArray();
    if (!array) return false;
    if (!array->empty()) {
        if (const auto* operation = (*array)[0].getString(); operation && *operation == name) return true;
    }
    return std::any_of(array->begin(), array->end(), [&](const auto& child) { return containsOperator(child, name); });
}

template <class T>
bool usesFeatureState(const PropertyValue<T>& value) {
    return value.match([](const Undefined&) { return false; },
                       [](const T&) { return false; },
                       [](const PropertyExpression<T>& expression) {
                           return containsOperator(expression.getExpression().serialize(), "feature-state");
                       });
}

template <class T>
float interpolationFactor(const PropertyValue<T>& value, float bucketZoom, float currentZoom) {
    return value.match([](const Undefined&) { return 0.0f; },
                       [](const T&) { return 0.0f; },
                       [&](const PropertyExpression<T>& expression) {
                           if (expression.isZoomConstant()) return 0.0f;
                           const auto zoom = expression.getUseIntegerZoom() ? std::floor(currentZoom) : currentZoom;
                           return std::clamp(
                               expression.interpolationFactor({bucketZoom, bucketZoom + 1.0f}, zoom), 0.0f, 1.0f);
                       });
}

template <class T>
T defaultValue(const plugin::PropertyDefinition& definition);

template <>
expression::Image defaultValue<expression::Image>(const plugin::PropertyDefinition& definition) {
    return expression::Image(*definition.defaultValue.getString());
}

template <>
bool defaultValue<bool>(const plugin::PropertyDefinition& definition) {
    return definition.defaultValue.getBool() && *definition.defaultValue.getBool();
}

template <>
float defaultValue<float>(const plugin::PropertyDefinition& definition) {
    return numericValue<float>(definition.defaultValue).value_or(0.0f);
}

template <>
std::array<float, 2> defaultValue<std::array<float, 2>>(const plugin::PropertyDefinition& definition) {
    std::array<float, 2> result{};
    if (const auto* array = definition.defaultValue.getArray(); array && array->size() == result.size()) {
        for (size_t i = 0; i < result.size(); ++i) result[i] = numericValue<float>((*array)[i]).value_or(0.0f);
    }
    return result;
}

template <>
Color defaultValue<Color>(const plugin::PropertyDefinition& definition) {
    std::array<float, 4> result{};
    if (const auto* array = definition.defaultValue.getArray(); array && array->size() == result.size()) {
        for (size_t i = 0; i < result.size(); ++i) result[i] = numericValue<float>((*array)[i]).value_or(0.0f);
    }
    return {result[0] * result[3], result[1] * result[3], result[2] * result[3], result[3]};
}

template <>
std::string defaultValue<std::string>(const plugin::PropertyDefinition& definition) {
    return definition.defaultValue.getString() ? *definition.defaultValue.getString() : std::string{};
}

// Dispatch descriptor types only at parsing/default construction. Evaluated values
// keep their native type until a C callback actually needs the ABI representation.
template <class Fn>
auto withPropertyType(mln_plugin_value_type type, Fn&& fn) {
    switch (type) {
        case MLN_PLUGIN_VALUE_IMAGE:
            return fn.template operator()<expression::Image>();
        case MLN_PLUGIN_VALUE_BOOLEAN:
            return fn.template operator()<bool>();
        case MLN_PLUGIN_VALUE_FLOAT:
            return fn.template operator()<float>();
        case MLN_PLUGIN_VALUE_FLOAT2:
            return fn.template operator()<std::array<float, 2>>();
        case MLN_PLUGIN_VALUE_COLOR:
            return fn.template operator()<Color>();
        case MLN_PLUGIN_VALUE_STRING:
            return fn.template operator()<std::string>();
    }
    throw std::logic_error("Invalid plugin property type");
}

template <class T>
T evaluateTyped(const PropertyValue<T>& value,
                const expression::EvaluationContext& context,
                const plugin::PropertyDefinition& definition) {
    const auto fallback = defaultValue<T>(definition);
    return value.match([&](const Undefined&) { return fallback; },
                       [&](const T& constant) { return constant; },
                       [&](const PropertyExpression<T>& expression) { return expression.evaluate(context, fallback); });
}

template <class T>
mln_plugin_value toPluginValue(const T& value, PluginPropertyValue::EvaluationStorage& storage) {
    mln_plugin_value result{};
    result.struct_size = sizeof(result);
    if constexpr (std::is_same_v<T, bool>) {
        result.type = MLN_PLUGIN_VALUE_BOOLEAN;
        result.data.boolean_value = value;
    } else if constexpr (std::is_same_v<T, float>) {
        result.type = MLN_PLUGIN_VALUE_FLOAT;
        result.data.float_value = value;
    } else if constexpr (std::is_same_v<T, std::array<float, 2>>) {
        result.type = MLN_PLUGIN_VALUE_FLOAT2;
        result.data.float2_value = {value[0], value[1]};
    } else if constexpr (std::is_same_v<T, Color>) {
        result.type = MLN_PLUGIN_VALUE_COLOR;
        result.data.color_value = {value.r, value.g, value.b, value.a};
    } else if constexpr (std::is_same_v<T, expression::Image>) {
        result.type = MLN_PLUGIN_VALUE_IMAGE;
        storage.string = value.id();
        result.data.string_value = {storage.string.data(), storage.string.size()};
    } else {
        result.type = MLN_PLUGIN_VALUE_STRING;
        storage.string = value;
        result.data.string_value = {storage.string.data(), storage.string.size()};
    }
    return result;
}

template <class T>
std::optional<PluginPropertyValue> convertTyped(const plugin::PropertyDefinition& definition,
                                                const conversion::Convertible& value,
                                                conversion::Error& error) {
    auto converted = conversion::convert<PropertyValue<T>>(
        value, error, definition.expressionCapabilities != MLN_PLUGIN_EXPRESSION_NONE, false);
    if (!converted) return std::nullopt;
    return PluginPropertyValue{PluginPropertyValue::TypedValue{std::move(*converted)}};
}

bool validateConstant(const plugin::PropertyDefinition& definition,
                      const PluginPropertyValue& property,
                      conversion::Error& error) {
    const auto converted = property.toStyleProperty();
    if (converted.getKind() != StyleProperty::Kind::Constant) return true;

    const auto& value = converted.getValue();
    if (definition.type == MLN_PLUGIN_VALUE_STRING && !definition.enumValues.empty()) {
        const auto* string = value.getString();
        if (!string || std::find(definition.enumValues.begin(), definition.enumValues.end(), *string) ==
                           definition.enumValues.end()) {
            error.message = "value is not allowed for plugin property '" + definition.name + "'";
            return false;
        }
    }

    const auto inRange = [&](double number) {
        return (!definition.minimum || number >= *definition.minimum) &&
               (!definition.maximum || number <= *definition.maximum);
    };
    if (definition.type == MLN_PLUGIN_VALUE_FLOAT) {
        const auto number = numericValue<double>(value);
        if (!number || !inRange(*number)) {
            error.message = "value is outside the allowed range for plugin property '" + definition.name + "'";
            return false;
        }
    }

    return true;
}

} // namespace

StyleProperty PluginPropertyValue::toStyleProperty() const {
    return std::visit(
        []<class T>(const PropertyValue<T>& typed) {
            if constexpr (std::is_same_v<T, expression::Image>) {
                if (typed.isConstant()) return StyleProperty{typed.asConstant().id(), StyleProperty::Kind::Constant};
            }
            return conversion::makeStyleProperty(typed);
        },
        value);
}

expression::Dependency PluginPropertyValue::getDependencies() const noexcept {
    return std::visit([](const auto& typed) { return typed.getDependencies(); }, value);
}

bool PluginPropertyValue::isUndefined() const noexcept {
    return std::visit([](const auto& typed) { return typed.isUndefined(); }, value);
}

bool PluginPropertyValue::isDataDriven() const noexcept {
    return std::visit([](const auto& typed) { return typed.isDataDriven(); }, value);
}

bool PluginPropertyValue::isZoomConstant() const noexcept {
    return std::visit([](const auto& typed) { return typed.isZoomConstant(); }, value);
}

bool PluginPropertyValue::usesFeatureState() const noexcept {
    return std::visit([](const auto& typed) { return style::usesFeatureState(typed); }, value);
}

float PluginPropertyValue::interpolationFactor(float bucketZoom, float currentZoom) const noexcept {
    return std::visit([&](const auto& typed) { return style::interpolationFactor(typed, bucketZoom, currentZoom); },
                      value);
}

mln_plugin_value PluginPropertyValue::evaluate(const expression::EvaluationContext& context,
                                               const plugin::PropertyDefinition& definition,
                                               EvaluationStorage& storage) const {
    return std::visit(
        [&](const auto& typed) { return toPluginValue(evaluateTyped(typed, context, definition), storage); }, value);
}

mln_plugin_value PluginPropertyValue::evaluate(float zoom,
                                               const GeometryTileFeature& feature,
                                               const FeatureState& state,
                                               const plugin::PropertyDefinition& definition,
                                               EvaluationStorage& storage,
                                               const std::set<std::string>* availableImages) const {
    return evaluate(expression::EvaluationContext(zoom, &feature, &state).withAvailableImages(availableImages),
                    definition,
                    storage);
}

mln_plugin_value PluginPropertyValue::evaluate(float zoom,
                                               const plugin::PropertyDefinition& definition,
                                               EvaluationStorage& storage,
                                               const std::set<std::string>* availableImages) const {
    return evaluate(expression::EvaluationContext(zoom).withAvailableImages(availableImages), definition, storage);
}

PluginTransitioningPropertyValue::PluginTransitioningPropertyValue(PluginPropertyValue input)
    : value(std::visit([](const auto& typed) -> TypedValue { return Transitioning{typed}; }, input.value)) {}

PluginTransitioningPropertyValue::PluginTransitioningPropertyValue(PluginPropertyValue input,
                                                                   PluginTransitioningPropertyValue prior,
                                                                   const TransitionOptions& transition,
                                                                   TimePoint now)
    : value(std::visit(
          [&]<class T>(const PropertyValue<T>& typed) -> TypedValue {
              return Transitioning<PropertyValue<T>>{
                  typed, std::get<Transitioning<PropertyValue<T>>>(std::move(prior.value)), transition, now};
          },
          input.value)) {}

PluginPropertyValue PluginTransitioningPropertyValue::evaluate(float zoom,
                                                               const plugin::PropertyDefinition& definition,
                                                               TimePoint now) {
    const PropertyEvaluationParameters parameters(zoom);
    return std::visit(
        [&]<class T>(const Transitioning<PropertyValue<T>>& typed) {
            if constexpr (std::is_same_v<T, expression::Image>) {
                if (typed.isUndefined())
                    return PluginPropertyValue{PluginPropertyValue::TypedValue{PropertyValue<T>{}}};
            }
            const auto evaluated = typed.evaluate(
                DataDrivenPropertyEvaluator<T>(parameters, defaultValue<T>(definition)), now);
            return evaluated.match([](const auto& result) {
                return PluginPropertyValue{PluginPropertyValue::TypedValue{PropertyValue<T>{result}}};
            });
        },
        value);
}

PluginPropertyValue defaultPluginPropertyValue(const plugin::PropertyDefinition& definition) {
    return withPropertyType(definition.type, [&]<class T>() {
        if constexpr (std::is_same_v<T, expression::Image>)
            return PluginPropertyValue{PluginPropertyValue::TypedValue{PropertyValue<T>{}}};
        else
            return PluginPropertyValue{PluginPropertyValue::TypedValue{PropertyValue<T>{defaultValue<T>(definition)}}};
    });
}

std::optional<PluginPropertyValue> convertPluginPropertyValue(const plugin::PropertyDefinition& definition,
                                                              const conversion::Convertible& value,
                                                              conversion::Error& error) {
    auto converted = withPropertyType(definition.type,
                                      [&]<class T>() { return convertTyped<T>(definition, value, error); });
    if (converted && !validateConstant(definition, *converted, error)) return std::nullopt;
    if (converted) {
        const auto dependencies = converted->getDependencies();
        const bool usesZoom = static_cast<bool>(dependencies & expression::Dependency::Zoom);
        const bool usesFeature = static_cast<bool>(dependencies & expression::Dependency::Feature);
        uint32_t required = MLN_PLUGIN_EXPRESSION_NONE;
        if (usesZoom && usesFeature) {
            required |= MLN_PLUGIN_EXPRESSION_COMPOSITE;
        } else if (usesZoom) {
            required |= MLN_PLUGIN_EXPRESSION_CAMERA;
        } else if (usesFeature) {
            required |= MLN_PLUGIN_EXPRESSION_FEATURE;
        }
        if (converted->usesFeatureState()) required |= MLN_PLUGIN_EXPRESSION_FEATURE_STATE;
        if ((required & ~definition.expressionCapabilities) != 0) {
            error.message = "expression dependencies are not supported for plugin property '" + definition.name + "'";
            return std::nullopt;
        }
    }
    return converted;
}

} // namespace style
} // namespace mln
