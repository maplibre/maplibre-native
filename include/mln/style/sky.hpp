// clang-format off

// This file is generated. Do not edit.

#pragma once

#include <mln/style/conversion.hpp>
#include <mln/style/property_value.hpp>
#include <mln/style/style_property.hpp>
#include <mln/style/transition_options.hpp>
#include <mln/style/types.hpp>
#include <mln/util/immutable.hpp>

namespace mln {
namespace style {

class SkyObserver;

class Sky {
public:
    Sky();
    ~Sky();

    // Dynamic properties
    std::optional<conversion::Error> setProperty(const std::string& name, const conversion::Convertible& value);
    StyleProperty getProperty(const std::string&) const;

    static float getDefaultAtmosphereBlend();
    PropertyValue<float> getAtmosphereBlend() const;
    void setAtmosphereBlend(PropertyValue<float>);
    void setAtmosphereBlendTransition(const TransitionOptions&);
    TransitionOptions getAtmosphereBlendTransition() const;

    static Color getDefaultFogColor();
    PropertyValue<Color> getFogColor() const;
    void setFogColor(PropertyValue<Color>);
    void setFogColorTransition(const TransitionOptions&);
    TransitionOptions getFogColorTransition() const;

    static float getDefaultFogGroundBlend();
    PropertyValue<float> getFogGroundBlend() const;
    void setFogGroundBlend(PropertyValue<float>);
    void setFogGroundBlendTransition(const TransitionOptions&);
    TransitionOptions getFogGroundBlendTransition() const;

    static Color getDefaultHorizonColor();
    PropertyValue<Color> getHorizonColor() const;
    void setHorizonColor(PropertyValue<Color>);
    void setHorizonColorTransition(const TransitionOptions&);
    TransitionOptions getHorizonColorTransition() const;

    static float getDefaultHorizonFogBlend();
    PropertyValue<float> getHorizonFogBlend() const;
    void setHorizonFogBlend(PropertyValue<float>);
    void setHorizonFogBlendTransition(const TransitionOptions&);
    TransitionOptions getHorizonFogBlendTransition() const;

    static Color getDefaultSkyColor();
    PropertyValue<Color> getSkyColor() const;
    void setSkyColor(PropertyValue<Color>);
    void setSkyColorTransition(const TransitionOptions&);
    TransitionOptions getSkyColorTransition() const;

    static float getDefaultSkyHorizonBlend();
    PropertyValue<float> getSkyHorizonBlend() const;
    void setSkyHorizonBlend(PropertyValue<float>);
    void setSkyHorizonBlendTransition(const TransitionOptions&);
    TransitionOptions getSkyHorizonBlendTransition() const;

    class Impl;
    Immutable<Impl> impl;
    explicit Sky(Immutable<Impl>);
    Mutable<Impl> mutableImpl() const;

    SkyObserver* observer = nullptr;
    void setObserver(SkyObserver*);
};

} // namespace style
} // namespace mln

// clang-format on
