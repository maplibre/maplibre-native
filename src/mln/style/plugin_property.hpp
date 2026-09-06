#pragma once

#include <mln/plugin/plugin_api.h>
#include <mln/style/property_value.hpp>
#include <mln/style/color_ramp_property_value.hpp>
#include <mln/style/style_property.hpp>
#include <mln/style/transition_options.hpp>
#include <mln/util/chrono.hpp>
#include <mln/util/color.hpp>

#include <array>
#include <map>
#include <memory>
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
                                    PropertyValue<std::vector<Color>>,
                                    ColorRampPropertyValue>;

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
    bool usesFeatureState() const noexcept;
    float interpolationFactor(float bucketZoom, float currentZoom) const noexcept;
    const ColorRampPropertyValue* colorRamp() const noexcept;

    mln_plugin_value evaluate(float zoom,
                              const GeometryTileFeature&,
                              const FeatureState&,
                              const plugin::PropertyDefinition&,
                              EvaluationStorage&) const;
    mln_plugin_value evaluate(float zoom, const plugin::PropertyDefinition&, EvaluationStorage&) const;
    PluginPropertyValue evaluateCamera(float zoom, const plugin::PropertyDefinition&) const;
    static PluginPropertyValue interpolate(const PluginPropertyValue&,
                                           const PluginPropertyValue&,
                                           float,
                                           const plugin::PropertyDefinition&);

    friend bool operator==(const PluginPropertyValue& lhs, const PluginPropertyValue& rhs) {
        return lhs.value == rhs.value;
    }
    friend bool operator!=(const PluginPropertyValue& lhs, const PluginPropertyValue& rhs) { return !(lhs == rhs); }

private:
    TypedValue value;
};

class PluginTransitioningPropertyValue {
public:
    explicit PluginTransitioningPropertyValue(PluginPropertyValue value_ = {})
        : value(std::move(value_)) {}
    PluginTransitioningPropertyValue(PluginPropertyValue,
                                     PluginTransitioningPropertyValue,
                                     const TransitionOptions&,
                                     TimePoint);

    PluginPropertyValue evaluate(float zoom, const plugin::PropertyDefinition&, TimePoint);
    bool hasTransition() const noexcept { return static_cast<bool>(prior); }

private:
    std::shared_ptr<PluginTransitioningPropertyValue> prior;
    TimePoint begin{};
    TimePoint end{};
    PluginPropertyValue value;
};

using PluginPropertyMap = std::map<std::string, PluginPropertyValue>;

PluginPropertyValue defaultPluginPropertyValue(const plugin::PropertyDefinition&);
std::optional<PluginPropertyValue> convertPluginPropertyValue(const plugin::PropertyDefinition&,
                                                              const conversion::Convertible&,
                                                              conversion::Error&);

} // namespace style
} // namespace mln
