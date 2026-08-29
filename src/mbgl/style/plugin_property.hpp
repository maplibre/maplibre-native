#pragma once

#include <mbgl/plugin/plugin_api.h>
#include <mbgl/style/property_value.hpp>
#include <mbgl/style/style_property.hpp>
#include <mbgl/util/color.hpp>

#include <array>
#include <variant>

namespace mbgl {
class GeometryTileFeature;
namespace plugin {
struct PropertyDefinition;
}
namespace style {
namespace conversion {
class Convertible;
struct Error;
}

class PluginPropertyValue {
public:
    using TypedValue = std::variant<PropertyValue<bool>,
                                    PropertyValue<float>,
                                    PropertyValue<std::array<float, 2>>,
                                    PropertyValue<Color>,
                                    PropertyValue<std::string>>;

    PluginPropertyValue() = default;
    explicit PluginPropertyValue(TypedValue value_)
        : value(std::move(value_)) {}

    StyleProperty toStyleProperty() const;
    expression::Dependency getDependencies() const noexcept;
    bool isDataDriven() const noexcept;
    bool isZoomConstant() const noexcept;

    mln_plugin_value evaluate(float zoom,
                              const GeometryTileFeature&,
                              const FeatureState&,
                              const plugin::PropertyDefinition&,
                              std::string& stringStorage) const;

    friend bool operator==(const PluginPropertyValue& lhs, const PluginPropertyValue& rhs) {
        return lhs.value == rhs.value;
    }
    friend bool operator!=(const PluginPropertyValue& lhs, const PluginPropertyValue& rhs) { return !(lhs == rhs); }

private:
    TypedValue value;
};

PluginPropertyValue defaultPluginPropertyValue(const plugin::PropertyDefinition&);
std::optional<PluginPropertyValue> convertPluginPropertyValue(const plugin::PropertyDefinition&,
                                                              const conversion::Convertible&,
                                                              conversion::Error&);

} // namespace style
} // namespace mbgl
