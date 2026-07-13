// clang-format off

// This file is generated. Do not edit.

#pragma once

#include <mbgl/style/color_ramp_property_value.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/filter.hpp>
#include <mbgl/style/property_value.hpp>
#include <mbgl/util/color.hpp>

namespace mbgl {
namespace style {

class TransitionOptions;

class ColorReliefLayer final : public Layer {
public:
    ColorReliefLayer(const std::string& layerID, const std::string& sourceID);
    ~ColorReliefLayer() override;

    // Paint properties

    static ColorRampPropertyValue getDefaultColorReliefColor();
    const ColorRampPropertyValue& getColorReliefColor() const;
    void setColorReliefColor(const ColorRampPropertyValue&);
    void setColorReliefColorTransition(const TransitionOptions&);
    TransitionOptions getColorReliefColorTransition() const;

    static PropertyValue<float> getDefaultColorReliefOpacity();
    const PropertyValue<float>& getColorReliefOpacity() const;
    void setColorReliefOpacity(const PropertyValue<float>&);
    void setColorReliefOpacityTransition(const TransitionOptions&);
    TransitionOptions getColorReliefOpacityTransition() const;

    // Private implementation

    class Impl;
    const Impl& impl() const;

    Mutable<Impl> mutableImpl() const;
    ColorReliefLayer(Immutable<Impl>);
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
