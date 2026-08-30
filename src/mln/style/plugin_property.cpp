#include <mln/style/plugin_property.hpp>

#include <mln/plugin/plugin_registry.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/expression/expression.hpp>
#include <mln/tile/geometry_tile_data.hpp>

#include <algorithm>

namespace mln {
namespace style {
namespace {

template <class T>
T defaultValue(const plugin::PropertyDefinition& definition);

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
    return {result[0], result[1], result[2], result[3]};
}

template <>
std::string defaultValue<std::string>(const plugin::PropertyDefinition& definition) {
    return definition.defaultValue.getString() ? *definition.defaultValue.getString() : std::string{};
}

template <>
std::vector<float> defaultValue<std::vector<float>>(const plugin::PropertyDefinition& definition) {
    std::vector<float> result;
    if (const auto* array = definition.defaultValue.getArray()) {
        result.reserve(array->size());
        for (const auto& item : *array) result.push_back(numericValue<float>(item).value_or(0.0f));
    }
    return result;
}

template <>
std::vector<Color> defaultValue<std::vector<Color>>(const plugin::PropertyDefinition& definition) {
    std::vector<Color> result;
    if (const auto* array = definition.defaultValue.getArray()) {
        result.reserve(array->size());
        for (const auto& item : *array) {
            const auto* components = item.getArray();
            if (!components || components->size() != 4) continue;
            result.emplace_back(numericValue<float>((*components)[0]).value_or(0.0f),
                                numericValue<float>((*components)[1]).value_or(0.0f),
                                numericValue<float>((*components)[2]).value_or(0.0f),
                                numericValue<float>((*components)[3]).value_or(0.0f));
        }
    }
    return result;
}

template <class T>
T evaluateTyped(const PropertyValue<T>& value,
                float zoom,
                const GeometryTileFeature& feature,
                const FeatureState& state,
                const plugin::PropertyDefinition& definition) {
    const auto fallback = defaultValue<T>(definition);
    return value.match([&](const Undefined&) { return fallback; },
                       [&](const T& constant) { return constant; },
                       [&](const PropertyExpression<T>& expression) {
                           return expression.evaluate(expression::EvaluationContext(zoom, &feature, &state), fallback);
                       });
}

template <class T>
T evaluateCameraTyped(const PropertyValue<T>& value,
                      float zoom,
                      const plugin::PropertyDefinition& definition) {
    const auto fallback = defaultValue<T>(definition);
    return value.match([&](const Undefined&) { return fallback; },
                       [&](const T& constant) { return constant; },
                       [&](const PropertyExpression<T>& expression) {
                           return expression.evaluate(expression::EvaluationContext(zoom), fallback);
                       });
}

template <class T>
std::optional<PluginPropertyValue> convertTyped(const plugin::PropertyDefinition& definition,
                                                const conversion::Convertible& value,
                                                conversion::Error& error) {
    auto converted = conversion::convert<PropertyValue<T>>(value, error, definition.supportsExpressions, false);
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

    if (definition.type == MLN_PLUGIN_VALUE_FLOAT_ARRAY || definition.type == MLN_PLUGIN_VALUE_COLOR_ARRAY) {
        const auto* array = value.getArray();
        if (!array) return true;
        if (definition.maximumArrayLength && array->size() > definition.maximumArrayLength) {
            error.message = "value has too many entries for plugin property '" + definition.name + "'";
            return false;
        }
        if (definition.type == MLN_PLUGIN_VALUE_FLOAT_ARRAY) {
            for (const auto& item : *array) {
                const auto number = numericValue<double>(item);
                if (!number || !inRange(*number)) {
                    error.message =
                        "array entry is outside the allowed range for plugin property '" + definition.name + "'";
                    return false;
                }
            }
        }
    }
    return true;
}

} // namespace

StyleProperty PluginPropertyValue::toStyleProperty() const {
    return std::visit([](const auto& typed) { return conversion::makeStyleProperty(typed); }, value);
}

expression::Dependency PluginPropertyValue::getDependencies() const noexcept {
    return std::visit([](const auto& typed) { return typed.getDependencies(); }, value);
}

bool PluginPropertyValue::isDataDriven() const noexcept {
    return std::visit([](const auto& typed) { return typed.isDataDriven(); }, value);
}

bool PluginPropertyValue::isZoomConstant() const noexcept {
    return std::visit([](const auto& typed) { return typed.isZoomConstant(); }, value);
}

mln_plugin_value PluginPropertyValue::evaluate(float zoom,
                                               const GeometryTileFeature& feature,
                                               const FeatureState& state,
                                               const plugin::PropertyDefinition& definition,
                                               EvaluationStorage& storage) const {
    mln_plugin_value result{};
    result.struct_size = sizeof(result);
    result.type = definition.type;
    switch (definition.type) {
        case MLN_PLUGIN_VALUE_BOOLEAN:
            result.data.boolean_value = evaluateTyped(
                std::get<PropertyValue<bool>>(value), zoom, feature, state, definition);
            break;
        case MLN_PLUGIN_VALUE_FLOAT:
            result.data.float_value = evaluateTyped(
                std::get<PropertyValue<float>>(value), zoom, feature, state, definition);
            break;
        case MLN_PLUGIN_VALUE_FLOAT2: {
            const auto evaluated = evaluateTyped(
                std::get<PropertyValue<std::array<float, 2>>>(value), zoom, feature, state, definition);
            result.data.float2_value = {evaluated[0], evaluated[1]};
            break;
        }
        case MLN_PLUGIN_VALUE_COLOR: {
            const auto evaluated = evaluateTyped(
                std::get<PropertyValue<Color>>(value), zoom, feature, state, definition);
            result.data.color_value = {evaluated.r, evaluated.g, evaluated.b, evaluated.a};
            break;
        }
        case MLN_PLUGIN_VALUE_STRING:
            storage.string = evaluateTyped(
                std::get<PropertyValue<std::string>>(value), zoom, feature, state, definition);
            result.data.string_value = {storage.string.data(), storage.string.size()};
            break;
        case MLN_PLUGIN_VALUE_FLOAT_ARRAY:
            storage.floats = evaluateTyped(
                std::get<PropertyValue<std::vector<float>>>(value), zoom, feature, state, definition);
            result.data.float_array_value = {storage.floats.data(), storage.floats.size()};
            break;
        case MLN_PLUGIN_VALUE_COLOR_ARRAY: {
            const auto evaluated = evaluateTyped(
                std::get<PropertyValue<std::vector<Color>>>(value), zoom, feature, state, definition);
            storage.colors.clear();
            storage.colors.reserve(evaluated.size());
            for (const auto& color : evaluated) storage.colors.push_back({color.r, color.g, color.b, color.a});
            result.data.color_array_value = {storage.colors.data(), storage.colors.size()};
            break;
        }
    }
    return result;
}

mln_plugin_value PluginPropertyValue::evaluate(float zoom,
                                               const plugin::PropertyDefinition& definition,
                                               EvaluationStorage& storage) const {
    mln_plugin_value result{};
    result.struct_size = sizeof(result);
    result.type = definition.type;
    switch (definition.type) {
        case MLN_PLUGIN_VALUE_BOOLEAN:
            result.data.boolean_value = evaluateCameraTyped(std::get<PropertyValue<bool>>(value), zoom, definition);
            break;
        case MLN_PLUGIN_VALUE_FLOAT:
            result.data.float_value = evaluateCameraTyped(std::get<PropertyValue<float>>(value), zoom, definition);
            break;
        case MLN_PLUGIN_VALUE_FLOAT2: {
            const auto evaluated = evaluateCameraTyped(
                std::get<PropertyValue<std::array<float, 2>>>(value), zoom, definition);
            result.data.float2_value = {evaluated[0], evaluated[1]};
            break;
        }
        case MLN_PLUGIN_VALUE_COLOR: {
            const auto evaluated = evaluateCameraTyped(std::get<PropertyValue<Color>>(value), zoom, definition);
            result.data.color_value = {evaluated.r, evaluated.g, evaluated.b, evaluated.a};
            break;
        }
        case MLN_PLUGIN_VALUE_STRING:
            storage.string = evaluateCameraTyped(std::get<PropertyValue<std::string>>(value), zoom, definition);
            result.data.string_value = {storage.string.data(), storage.string.size()};
            break;
        case MLN_PLUGIN_VALUE_FLOAT_ARRAY:
            storage.floats = evaluateCameraTyped(
                std::get<PropertyValue<std::vector<float>>>(value), zoom, definition);
            result.data.float_array_value = {storage.floats.data(), storage.floats.size()};
            break;
        case MLN_PLUGIN_VALUE_COLOR_ARRAY: {
            const auto evaluated = evaluateCameraTyped(
                std::get<PropertyValue<std::vector<Color>>>(value), zoom, definition);
            storage.colors.clear();
            storage.colors.reserve(evaluated.size());
            for (const auto& color : evaluated) storage.colors.push_back({color.r, color.g, color.b, color.a});
            result.data.color_array_value = {storage.colors.data(), storage.colors.size()};
            break;
        }
    }
    return result;
}

PluginPropertyValue defaultPluginPropertyValue(const plugin::PropertyDefinition& definition) {
    switch (definition.type) {
        case MLN_PLUGIN_VALUE_BOOLEAN:
            return PluginPropertyValue{
                PluginPropertyValue::TypedValue{PropertyValue<bool>{defaultValue<bool>(definition)}}};
        case MLN_PLUGIN_VALUE_FLOAT:
            return PluginPropertyValue{
                PluginPropertyValue::TypedValue{PropertyValue<float>{defaultValue<float>(definition)}}};
        case MLN_PLUGIN_VALUE_FLOAT2:
            return PluginPropertyValue{PluginPropertyValue::TypedValue{
                PropertyValue<std::array<float, 2>>{defaultValue<std::array<float, 2>>(definition)}}};
        case MLN_PLUGIN_VALUE_COLOR:
            return PluginPropertyValue{
                PluginPropertyValue::TypedValue{PropertyValue<Color>{defaultValue<Color>(definition)}}};
        case MLN_PLUGIN_VALUE_STRING:
            return PluginPropertyValue{
                PluginPropertyValue::TypedValue{PropertyValue<std::string>{defaultValue<std::string>(definition)}}};
        case MLN_PLUGIN_VALUE_FLOAT_ARRAY:
            return PluginPropertyValue{PluginPropertyValue::TypedValue{
                PropertyValue<std::vector<float>>{defaultValue<std::vector<float>>(definition)}}};
        case MLN_PLUGIN_VALUE_COLOR_ARRAY:
            return PluginPropertyValue{PluginPropertyValue::TypedValue{
                PropertyValue<std::vector<Color>>{defaultValue<std::vector<Color>>(definition)}}};
    }
    return {};
}

std::optional<PluginPropertyValue> convertPluginPropertyValue(const plugin::PropertyDefinition& definition,
                                                              const conversion::Convertible& value,
                                                              conversion::Error& error) {
    const bool arrayValue = definition.type == MLN_PLUGIN_VALUE_FLOAT_ARRAY ||
                            definition.type == MLN_PLUGIN_VALUE_COLOR_ARRAY;
    if (arrayValue && !definition.acceptsScalar && !isArray(value) && !isObject(value)) {
        error.message = "value must be an array for plugin property '" + definition.name + "'";
        return std::nullopt;
    }

    std::optional<PluginPropertyValue> converted;
    switch (definition.type) {
        case MLN_PLUGIN_VALUE_BOOLEAN:
            converted = convertTyped<bool>(definition, value, error);
            break;
        case MLN_PLUGIN_VALUE_FLOAT:
            converted = convertTyped<float>(definition, value, error);
            break;
        case MLN_PLUGIN_VALUE_FLOAT2:
            converted = convertTyped<std::array<float, 2>>(definition, value, error);
            break;
        case MLN_PLUGIN_VALUE_COLOR:
            converted = convertTyped<Color>(definition, value, error);
            break;
        case MLN_PLUGIN_VALUE_STRING:
            converted = convertTyped<std::string>(definition, value, error);
            break;
        case MLN_PLUGIN_VALUE_FLOAT_ARRAY:
            converted = convertTyped<std::vector<float>>(definition, value, error);
            break;
        case MLN_PLUGIN_VALUE_COLOR_ARRAY:
            converted = convertTyped<std::vector<Color>>(definition, value, error);
            break;
    }
    if (converted && !validateConstant(definition, *converted, error)) return std::nullopt;
    return converted;
}

} // namespace style
} // namespace mln
