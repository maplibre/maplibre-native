#pragma once

#include <mln/renderer/property_evaluator.hpp>
#include <mln/style/properties.hpp>
#include <mln/style/property_value.hpp>
#include <mln/style/sky.hpp>
#include <mln/util/color.hpp>

namespace mln {
namespace style {

template <class T>
class SkyProperty {
public:
    using TransitionableType = Transitionable<PropertyValue<T>>;
    using UnevaluatedType = Transitioning<PropertyValue<T>>;
    using EvaluatorType = PropertyEvaluator<T>;
    using PossiblyEvaluatedType = T;
    using Type = T;
    static constexpr bool IsDataDriven = false;
    static constexpr bool IsOverridable = false;
};

struct SkyAtmosphereBlend : SkyProperty<float> {
    static float defaultValue() { return 0.8f; }
};

struct SkyFogColor : SkyProperty<Color> {
    static Color defaultValue() { return Color::white(); }
};

struct SkyFogGroundBlend : SkyProperty<float> {
    static float defaultValue() { return 0.5f; }
};

struct SkyHorizonColor : SkyProperty<Color> {
    static Color defaultValue() { return Color::white(); }
};

struct SkyHorizonFogBlend : SkyProperty<float> {
    static float defaultValue() { return 0.8f; }
};

struct SkyColor : SkyProperty<Color> {
    static Color defaultValue() { return {136.0f / 255.0f, 198.0f / 255.0f, 252.0f / 255.0f, 1.0f}; }
};

struct SkyHorizonBlend : SkyProperty<float> {
    static float defaultValue() { return 0.8f; }
};

using SkyProperties = Properties<SkyAtmosphereBlend,
                                 SkyFogColor,
                                 SkyFogGroundBlend,
                                 SkyHorizonColor,
                                 SkyHorizonFogBlend,
                                 SkyColor,
                                 SkyHorizonBlend>;

class Sky::Impl {
public:
    SkyProperties::Transitionable properties;
};

} // namespace style
} // namespace mln
