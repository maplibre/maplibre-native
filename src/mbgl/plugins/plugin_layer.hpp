#pragma once

//#include <mbgl/style/color_ramp_property_value.hpp>
#include <mbgl/style/layer.hpp>
//#include <mbgl/style/filter.hpp>
//#include <mbgl/style/property_value.hpp>
//#include <mbgl/util/color.hpp>

namespace mbgl::style {

class PluginLayer final : public Layer {
public:
    PluginLayer(const std::string& layerID, const std::string& sourceID);
    ~PluginLayer() override;

    // Private implementation
    class Impl;
    const Impl& impl() const;

    Mutable<Impl> mutableImpl() const;
    PluginLayer(Immutable<Impl>);
    std::unique_ptr<Layer> cloneRef(const std::string& id) const final;

protected:
    // Dynamic properties
    std::optional<conversion::Error> setPropertyInternal(const std::string& name, const conversion::Convertible& value) final;

    StyleProperty getProperty(const std::string& name) const final;
    Value serialize() const final;

    Mutable<Layer::Impl> mutableBaseImpl() const final;
};

} // namespace style
// } // namespace mbgl

