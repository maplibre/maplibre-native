#pragma once

#include <mln/plugin/plugin_api.h>
#include <mln/style/property_value.hpp>
#include <mln/style/style_property.hpp>
#include <mln/util/color.hpp>

#include <array>
#include <vector>
#include <variant>

namespace mln {
class GeometryTileFeature;
namespace plugin {
struct PropertyDefinition;
}
namespace style {
namespace conversion {
class Convertible;
struct Error;
} // namespace conversion

class PluginPropertyValue {
public:
    using TypedValue = std::variant<PropertyValue<bool>,
                                    PropertyValue<float>,
                                    PropertyValue<std::array<float, 2>>,
                                    PropertyValue<Color>,
                                    PropertyValue<std::string>,
                                    PropertyValue<std::vector<float>>,
                                    PropertyValue<std::vector<Color>>>;

    struct EvaluationStorage {
        std::string string;
        std::vector<float> floats;
        std::vector<mln_plugin_color> colors;
    };

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
                              EvaluationStorage&) const;
    mln_plugin_value evaluate(float zoom,
                              const plugin::PropertyDefinition&,
                              EvaluationStorage&) const;

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
} // namespace mln
