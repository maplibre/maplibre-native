#pragma once

// #include <mbgl/style/color_ramp_property_value.hpp>
#include <mbgl/style/layer.hpp>
// #include <mbgl/style/filter.hpp>
// #include <mbgl/style/property_value.hpp>
// #include <mbgl/util/color.hpp>

namespace mbgl::style {

class PluginLayer final : public Layer {
public:
    PluginLayer(const std::string& layerID
                , const std::string& sourceID
                , const style::LayerTypeInfo layerTypeInfo
                , const std::string & layerProperties
    
                //,const style::conversion::Convertible& layerProperties
    );
    ~PluginLayer() override;

    // Private implementation
    class Impl;
    const Impl& impl() const;

    Mutable<Impl> mutableImpl() const;
    PluginLayer(Immutable<Impl>);
    std::unique_ptr<Layer> cloneRef(const std::string& id) const final;

    
public:
    typedef std::function<void()> OnRenderLayer;
    void setRenderFunction(OnRenderLayer renderFunction) { _renderFunction = renderFunction; }

    typedef std::function<void()> OnUpdateLayer;
    void setUpdateFunction(OnUpdateLayer updateFunction) { _updateFunction = updateFunction; }

    typedef std::function<void(const std::string & properties)> OnUpdateLayerProperties;
    void setOnUpdateLayerPropertiesFunction(OnUpdateLayerProperties updateFunction) { _updateLayerPropertiesFunction = updateFunction; }

    // Don't love that these are here, would like to move to private but factory needs access (for now)
    OnRenderLayer _renderFunction;
    OnUpdateLayer _updateFunction;
    OnUpdateLayerProperties _updateLayerPropertiesFunction;

    
protected:
    // Dynamic properties
    //    std::optional<conversion::Error> setPropertyInternal(const std::string& name,
    //                                                         const conversion::Convertible& value) final;
    std::optional<conversion::Error> setPropertyInternal(const std::string& name,
                                                         const conversion::Convertible& value) final;

    StyleProperty getProperty(const std::string& name) const final;
    Value serialize() const final;

    Mutable<Layer::Impl> mutableBaseImpl() const final;
    

};

} // namespace mbgl::style
// } // namespace mbgl
