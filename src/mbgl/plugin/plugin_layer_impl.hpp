#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/plugin/plugin_layer.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/style/layout_property.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/shaders/attributes.hpp>
#include <mbgl/shaders/uniforms.hpp>
#include <mbgl/style/property_value.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/util/color.hpp>

#include <map>
#include <vector>

namespace mbgl {
namespace style {

using namespace conversion;

struct DataDrivenSingleFloatProperty : DataDrivenPaintProperty<float, attributes::width, uniforms::width> {
    static float defaultValue() { return 1.f; }
};

struct DataDrivenColorProperty : DataDrivenPaintProperty<mbgl::Color, attributes::color, uniforms::color> {
    static mbgl::Color defaultValue() { return mbgl::Color::black(); }
    static constexpr auto expressionType() { return expression::type::ColorType{}; };
    using EvaluatorType = DataDrivenPropertyEvaluator<Color, false>;
};

class PluginLayerProperty {
public:
    enum class PropertyType {
        Unknown,
        SingleFloat,
        Color
    };

public:
    PropertyType _propertyType = PropertyType::Unknown;
    std::string _propertyName;
    void setPropertyValue(const conversion::Convertible& value);

public:
    const PropertyValue<float>& getSingleFloat() const;
    void setSingleFloat(const PropertyValue<float>& value);
    float _defaultSingleFloatValue = 1.0;
    float _singleFloatValue = 0;
    PropertyValue<float> _singleFloatProperty;
    void setCurrentSingleFloatValue(float value);

    // Color
    const PropertyValue<mbgl::Color>& getColor() const;
    void setColor(const PropertyValue<mbgl::Color>& value);
    mbgl::Color _defaultColorValue = mbgl::Color::black();
    mbgl::Color _dataDrivenColorValue = mbgl::Color::black();
    PropertyValue<mbgl::Color> _dataDrivenColorProperty;
    void setCurrentColorValue(mbgl::Color value);

    // Return this property as json
    std::string asJSON();

private:
};

class PluginLayerPropertyManager {
public:
    PluginLayerProperty* getProperty(const std::string& propertyName);
    void addProperty(PluginLayerProperty* property);

    std::string propertiesAsJSON();

    std::vector<PluginLayerProperty*> getProperties();

private:
    std::map<std::string, PluginLayerProperty*> _properties;
};

class PluginLayer::Impl : public Layer::Impl {
public:
    Impl(std::string layerID, std::string sourceID, LayerTypeInfo layerTypeInfo, const std::string& layerProperties);

    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    const LayerTypeInfo* getTypeInfo() const noexcept final { return &_layerTypeInfo; }

    void setRenderFunction(OnRenderLayer renderFunction) { _renderFunction = renderFunction; }

    void setUpdateFunction(OnUpdateLayer updateFunction) { _updateFunction = updateFunction; }

    void setUpdatePropertiesFunction(OnUpdateLayerProperties updateLayerPropertiesFunction) {
        _updateLayerPropertiesFunction = updateLayerPropertiesFunction;
    }

    void setFeatureCollectionLoadedFunction(OnFeatureCollectionLoaded featureCollectionLoadedFunction) {
        _featureCollectionLoadedFunction = featureCollectionLoadedFunction;
    }

    void setFeatureCollectionUnloadedFunction(OnFeatureCollectionUnloaded featureCollectionUnloadedFunction) {
        _featureCollectionUnloadedFunction = featureCollectionUnloadedFunction;
    }

    //! The property manager handles all of the custom properties for this layer type / instance
    PluginLayerPropertyManager _propertyManager;

    //! Optional: Called when the layer is expected to render itself.
    OnRenderLayer _renderFunction;

    //! Optional: Called when the layer is expected to update it's animations/etc.
    // TODO: Does this need to be here or can it be done via the render function.  Potentially, we could
    // have this method called on a background thread/etc or use another way to parallalize work
    OnUpdateLayer _updateFunction;

    //! Optional: Called when the layer properties change.  The properties are passed as JSON for now
    OnUpdateLayerProperties _updateLayerPropertiesFunction;

    //! Optional: Called when a feature collection is loaded
    OnFeatureCollectionLoaded _featureCollectionLoadedFunction;

    //! Optional: Called when a feature colleciton is unloaded from the scene (tile goes out of scene/etc)
    OnFeatureCollectionUnloaded _featureCollectionUnloadedFunction;

private:
    LayerTypeInfo _layerTypeInfo;
    std::string _layerProperties;
};

} // namespace style
} // namespace mbgl
