#pragma once

#include <mln/plugin/plugin_api.h>
#include <mln/style/property_value.hpp>
#include <mln/style/expression/image.hpp>
#include <mln/style/properties.hpp>
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
    using TypedValue = std::variant<PropertyValue<float>,
                                    PropertyValue<std::array<float, 2>>,
                                    PropertyValue<Color>,
                                    PropertyValue<std::string>,
                                    PropertyValue<bool>,
                                    PropertyValue<expression::Image>>;

    struct EvaluationStorage {
        std::string string;
    };

    PluginPropertyValue() = default;
    explicit PluginPropertyValue(TypedValue value_)
        : value(std::move(value_)) {}

    StyleProperty toStyleProperty() const;
    expression::Dependency getDependencies() const noexcept;
    bool isDataDriven() const noexcept;
    bool isUndefined() const noexcept;
    bool isZoomConstant() const noexcept;
    bool usesFeatureState() const noexcept;
    float interpolationFactor(float bucketZoom, float currentZoom) const noexcept;

    mln_plugin_value evaluate(float zoom,
                              const GeometryTileFeature&,
                              const FeatureState&,
                              const plugin::PropertyDefinition&,
                              EvaluationStorage&,
                              const std::set<std::string>* availableImages = nullptr) const;
    mln_plugin_value evaluate(float zoom,
                              const plugin::PropertyDefinition&,
                              EvaluationStorage&,
                              const std::set<std::string>* availableImages = nullptr) const;

    friend bool operator==(const PluginPropertyValue& lhs, const PluginPropertyValue& rhs) {
        return lhs.value == rhs.value;
    }
    friend bool operator!=(const PluginPropertyValue& lhs, const PluginPropertyValue& rhs) { return !(lhs == rhs); }

private:
    friend class PluginTransitioningPropertyValue;
    mln_plugin_value evaluate(const expression::EvaluationContext&,
                              const plugin::PropertyDefinition&,
                              EvaluationStorage&) const;
    TypedValue value;
};

class PluginTransitioningPropertyValue {
public:
    explicit PluginTransitioningPropertyValue(PluginPropertyValue = {});
    PluginTransitioningPropertyValue(PluginPropertyValue,
                                     PluginTransitioningPropertyValue,
                                     const TransitionOptions&,
                                     TimePoint);

    PluginPropertyValue evaluate(float zoom, const plugin::PropertyDefinition&, TimePoint);
    bool hasTransition() const noexcept {
        return std::visit([](const auto& typed) { return typed.hasTransition(); }, value);
    }

private:
    using TypedValue = std::variant<Transitioning<PropertyValue<float>>,
                                    Transitioning<PropertyValue<std::array<float, 2>>>,
                                    Transitioning<PropertyValue<Color>>,
                                    Transitioning<PropertyValue<std::string>>,
                                    Transitioning<PropertyValue<bool>>,
                                    Transitioning<PropertyValue<expression::Image>>>;
    TypedValue value;
};

using PluginPropertyMap = std::map<std::string, PluginPropertyValue>;

PluginPropertyValue defaultPluginPropertyValue(const plugin::PropertyDefinition&);
std::optional<PluginPropertyValue> convertPluginPropertyValue(const plugin::PropertyDefinition&,
                                                              const conversion::Convertible&,
                                                              conversion::Error&);

} // namespace style
} // namespace mln
