#include <mln/style/plugin_property.hpp>

#include <mln/plugin/plugin_registry.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/expression/expression.hpp>
#include <mln/tile/geometry_tile_data.hpp>

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
std::optional<PluginPropertyValue> convertTyped(const plugin::PropertyDefinition& definition,
                                                const conversion::Convertible& value,
                                                conversion::Error& error) {
    auto converted = conversion::convert<PropertyValue<T>>(value, error, definition.supportsExpressions, false);
    if (!converted) return std::nullopt;
    return PluginPropertyValue{PluginPropertyValue::TypedValue{std::move(*converted)}};
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
                                               std::string& stringStorage) const {
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
            stringStorage = evaluateTyped(
                std::get<PropertyValue<std::string>>(value), zoom, feature, state, definition);
            result.data.string_value = {stringStorage.data(), stringStorage.size()};
            break;
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
    }
    return {};
}

std::optional<PluginPropertyValue> convertPluginPropertyValue(const plugin::PropertyDefinition& definition,
                                                              const conversion::Convertible& value,
                                                              conversion::Error& error) {
    switch (definition.type) {
        case MLN_PLUGIN_VALUE_BOOLEAN:
            return convertTyped<bool>(definition, value, error);
        case MLN_PLUGIN_VALUE_FLOAT:
            return convertTyped<float>(definition, value, error);
        case MLN_PLUGIN_VALUE_FLOAT2:
            return convertTyped<std::array<float, 2>>(definition, value, error);
        case MLN_PLUGIN_VALUE_COLOR:
            return convertTyped<Color>(definition, value, error);
        case MLN_PLUGIN_VALUE_STRING:
            return convertTyped<std::string>(definition, value, error);
    }
    error.message = "unsupported plugin property type";
    return std::nullopt;
}

} // namespace style
} // namespace mln
