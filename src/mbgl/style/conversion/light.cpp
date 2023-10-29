#include <mbgl/style/conversion/light.hpp>
#include <mbgl/style/conversion/position.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/style/conversion/transition_options.hpp>
#include <mbgl/style/conversion_impl.hpp>

namespace mbgl {
namespace style {
namespace conversion {

std::optional<Light> Converter<Light>::operator()(const Convertible& value, Error& error) const {
    if (!isObject(value)) {
        error.message = "light must be an object";
        return std::nullopt;
    }

    Light light;

    const auto anchor = objectMember(value, "anchor");
    if (anchor) {
        auto convertedAnchor = convert<PropertyValue<LightAnchorType>>(*anchor, error, false, false);
        if (convertedAnchor) {
            light.setAnchor(*convertedAnchor);
        } else {
            return std::nullopt;
        }
    }

    const auto anchorTransition = objectMember(value, "anchor-transition");
    if (anchorTransition) {
        auto transition = convert<TransitionOptions>(*anchorTransition, error);
        if (transition) {
            light.setAnchorTransition(*transition);
        } else {
            return std::nullopt;
        }
    }

    const auto color = objectMember(value, "color");
    if (color) {
        auto convertedColor = convert<PropertyValue<Color>>(*color, error, false, false);

        if (convertedColor) {
            light.setColor(*convertedColor);
        } else {
            return std::nullopt;
        }
    }

    const auto colorTransition = objectMember(value, "color-transition");
    if (colorTransition) {
        auto transition = convert<TransitionOptions>(*colorTransition, error);
        if (transition) {
            light.setColorTransition(*transition);
        } else {
            return std::nullopt;
        }
    }

    const auto position = objectMember(value, "position");
    if (position) {
        auto convertedPosition = convert<PropertyValue<Position>>(*position, error, false, false);

        if (convertedPosition) {
            light.setPosition(*convertedPosition);
        } else {
            return std::nullopt;
        }
    }

    const auto positionTransition = objectMember(value, "position-transition");
    if (positionTransition) {
        auto transition = convert<TransitionOptions>(*positionTransition, error);
        if (transition) {
            light.setPositionTransition(*transition);
        } else {
            return std::nullopt;
        }
    }

    const auto intensity = objectMember(value, "intensity");
    if (intensity) {
        auto convertedIntensity = convert<PropertyValue<float>>(*intensity, error, false, false);

        if (convertedIntensity) {
            light.setIntensity(*convertedIntensity);
        } else {
            return std::nullopt;
        }
    }

    const auto intensityTransition = objectMember(value, "intensity-transition");
    if (intensityTransition) {
        auto transition = convert<TransitionOptions>(*intensityTransition, error);
        if (transition) {
            light.setIntensityTransition(*transition);
        } else {
            return std::nullopt;
        }
    }

    return {std::move(light)};
}

} // namespace conversion
} // namespace style
} // namespace mbgl
