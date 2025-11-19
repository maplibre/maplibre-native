// clang-format off

// This file is generated. Do not edit.

#pragma once

#include <mbgl/style/layer.hpp>
#include <mbgl/style/filter.hpp>
#include <mbgl/style/property_value.hpp>
#include <mbgl/util/color.hpp>

namespace mbgl {
namespace style {

class TransitionOptions;

class HillshadeLayer final : public Layer {
public:
    HillshadeLayer(const std::string& layerID, const std::string& sourceID);
    ~HillshadeLayer() override;

    // Paint properties

    static PropertyValue<Color> getDefaultHillshadeAccentColor();
    const PropertyValue<Color>& getHillshadeAccentColor() const;
    void setHillshadeAccentColor(const PropertyValue<Color>&);
    void setHillshadeAccentColorTransition(const TransitionOptions&);
    TransitionOptions getHillshadeAccentColorTransition() const;

    static PropertyValue<float> getDefaultHillshadeExaggeration();
    const PropertyValue<float>& getHillshadeExaggeration() const;
    void setHillshadeExaggeration(const PropertyValue<float>&);
    void setHillshadeExaggerationTransition(const TransitionOptions&);
    TransitionOptions getHillshadeExaggerationTransition() const;

    static PropertyValue<std::vector<Color>> getDefaultHillshadeHighlightColor();
    const PropertyValue<std::vector<Color>>& getHillshadeHighlightColor() const;
    void setHillshadeHighlightColor(const PropertyValue<std::vector<Color>>&);
    void setHillshadeHighlightColorTransition(const TransitionOptions&);
    TransitionOptions getHillshadeHighlightColorTransition() const;

    static PropertyValue<std::vector<float>> getDefaultHillshadeIlluminationAltitude();
    const PropertyValue<std::vector<float>>& getHillshadeIlluminationAltitude() const;
    void setHillshadeIlluminationAltitude(const PropertyValue<std::vector<float>>&);
    void setHillshadeIlluminationAltitudeTransition(const TransitionOptions&);
    TransitionOptions getHillshadeIlluminationAltitudeTransition() const;

    static PropertyValue<HillshadeIlluminationAnchorType> getDefaultHillshadeIlluminationAnchor();
    const PropertyValue<HillshadeIlluminationAnchorType>& getHillshadeIlluminationAnchor() const;
    void setHillshadeIlluminationAnchor(const PropertyValue<HillshadeIlluminationAnchorType>&);
    void setHillshadeIlluminationAnchorTransition(const TransitionOptions&);
    TransitionOptions getHillshadeIlluminationAnchorTransition() const;

    static PropertyValue<std::vector<float>> getDefaultHillshadeIlluminationDirection();
    const PropertyValue<std::vector<float>>& getHillshadeIlluminationDirection() const;
    void setHillshadeIlluminationDirection(const PropertyValue<std::vector<float>>&);
    void setHillshadeIlluminationDirectionTransition(const TransitionOptions&);
    TransitionOptions getHillshadeIlluminationDirectionTransition() const;

    static PropertyValue<HillshadeMethodType> getDefaultHillshadeMethod();
    const PropertyValue<HillshadeMethodType>& getHillshadeMethod() const;
    void setHillshadeMethod(const PropertyValue<HillshadeMethodType>&);
    void setHillshadeMethodTransition(const TransitionOptions&);
    TransitionOptions getHillshadeMethodTransition() const;

    static PropertyValue<std::vector<Color>> getDefaultHillshadeShadowColor();
    const PropertyValue<std::vector<Color>>& getHillshadeShadowColor() const;
    void setHillshadeShadowColor(const PropertyValue<std::vector<Color>>&);
    void setHillshadeShadowColorTransition(const TransitionOptions&);
    TransitionOptions getHillshadeShadowColorTransition() const;

    // Private implementation

    class Impl;
    const Impl& impl() const;

    Mutable<Impl> mutableImpl() const;
    HillshadeLayer(Immutable<Impl>);
    std::unique_ptr<Layer> cloneRef(const std::string& id) const final;

protected:
    // Dynamic properties
    std::optional<conversion::Error> setPropertyInternal(const std::string& name, const conversion::Convertible& value) final;

    StyleProperty getProperty(const std::string& name) const final;
    Value serialize() const final;

    Mutable<Layer::Impl> mutableBaseImpl() const final;
};

} // namespace style
} // namespace mbgl

// clang-format on
