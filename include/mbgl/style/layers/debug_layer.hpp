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

class DebugLayer final : public Layer {
public:
    DebugLayer(const std::string& layerID);
    ~DebugLayer() override;

    // Paint properties

    static PropertyValue<Color> getDefaultBorderColor();
    const PropertyValue<Color>& getBorderColor() const;
    void setBorderColor(const PropertyValue<Color>&);
    void setBorderColorTransition(const TransitionOptions&);
    TransitionOptions getBorderColorTransition() const;

    // Private implementation

    class Impl;
    const Impl& impl() const;

    Mutable<Impl> mutableImpl() const;
    DebugLayer(Immutable<Impl>);
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
