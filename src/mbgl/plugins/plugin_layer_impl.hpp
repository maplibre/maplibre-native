#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/plugins/plugin_layer.hpp>
// #include <mbgl/style/layers/plugin_layer_properties.hpp>
#include <mbgl/style/conversion_impl.hpp>



// ---------------------------------------------------
// Properties stuff
#include <map>
#include <mbgl/style/types.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/style/layers/heatmap_layer.hpp>
#include <mbgl/style/layout_property.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/uniforms.hpp>

#include <mbgl/style/property_value.hpp>
#include <mbgl/style/conversion/property_value.hpp>

/*
#include <mbgl/style/types.hpp>
#include <mbgl/style/layer_properties.hpp>
//#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/filter.hpp>
#include <mbgl/style/property_value.hpp>
#include <mbgl/style/layout_property.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/uniforms.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_layer.hpp>
*/
// ---------------------------------------------------


namespace mbgl {
namespace style {

using namespace conversion;

struct SingleFloatProperty : DataDrivenPaintProperty<float, attributes::width, uniforms::width> {
    static float defaultValue() { return 1.f; }
};


struct Scale : DataDrivenPaintProperty<float, attributes::width, uniforms::width> {
    static float defaultValue() { return 1.f; }
};

class PluginPaintProperties : public Properties<
    Scale
>{};

template <>
struct Converter<Scale> {
    std::optional<Scale> operator()(const Convertible& value,
                                    Error& error,
                                    bool /* allowDataExpressions */ = true,
                                    bool /* convertTokens */ = false) const;
};

template <>
struct Converter<PropertyValue<Scale>> {
    std::optional<PropertyValue<Scale>> operator()(const Convertible& value,
                                    Error& error,
                                    bool /* allowDataExpressions */ = true,
                                    bool /* convertTokens */ = false) const;
};


//
//template std::optional<PropertyValue<Scale>> Converter<PropertyValue<Scale>>::operator()(conversion::Convertible const&,
//                                                                                         conversion::Error&,
//                                                                                         bool,
//                                                                                         bool) const;


class PluginLayerProperty {
public:
    std::string _propertyName;
    void setPropertyValue(const conversion::Convertible& value);
public:
    const PropertyValue<float> &getSingleFloat() const;
    void setSingleFloat(const PropertyValue<float> &value);
    
    float _singleFloatValue = 0;
    PropertyValue<float> _singleFloatProperty;
    void setCurrentSingleFloatValue(float value);
    
    
    
    const PropertyValue<float>& getScale() const;
    void setScale(const PropertyValue<float>& value);
    void setScaleTransition(const TransitionOptions& options);
    TransitionOptions getScaleTransition() const;
    
    const PropertyValue<Scale>& getScale2() const;
    void setScale2(const PropertyValue<Scale>& value);
    void setScale2Transition(const TransitionOptions& options);
    TransitionOptions getScale2Transition() const;
    
    
private:
    PluginPaintProperties::Transitionable paint;

    
    
};

class PluginLayerPropertyManager {
public:
    PluginLayerProperty *getProperty(const std::string & propertyName);
    void addProperty(PluginLayerProperty *property);

private:
    std::map<std::string, PluginLayerProperty *> _properties;

};


class PluginLayer::Impl : public Layer::Impl {
public:
    Impl(std::string layerID, std::string sourceID, LayerTypeInfo layerTypeInfo, const std::string& layerProperties
         //,const style::conversion::Convertible& layerProperties
    );

    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    const LayerTypeInfo* getTypeInfo() const noexcept final {
        return &_layerTypeInfo;
        // TODO: Return the right thing here
        // return nullptr;
    }

    void setRenderFunction(OnRenderLayer renderFunction) {
        _renderFunction = renderFunction;
    }
    
    void setUpdateFunction(OnUpdateLayer updateFunction) {
        _updateFunction = updateFunction;
    }
    
    void setUpdatePropertiesFunction(OnUpdateLayerProperties updateLayerPropertiesFunction) { _updateLayerPropertiesFunction = updateLayerPropertiesFunction; }

    
    PluginLayerPropertyManager _propertyManager;


    OnRenderLayer _renderFunction;
    OnUpdateLayer _updateFunction;
    OnUpdateLayerProperties _updateLayerPropertiesFunction;

private:
    LayerTypeInfo _layerTypeInfo;
    std::string _layerProperties;
    // style::conversion::Convertible _layerProperties;

    // HeatmapPaintProperties::Transitionable paint;

    // Not needed for this
    // DECLARE_LAYER_TYPE_INFO;

    //    const LayerTypeInfo* getTypeInfo() const noexcept final {
    //        return staticTypeInfo();
    //    }
    //    static const LayerTypeInfo* staticTypeInfo() noexcept;
};

} // namespace style
} // namespace mbgl
