#pragma once

#include <mln/style/layer_impl.hpp>
#include <mln/plugin/plugin_layer.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/types.hpp>
#include <mln/style/layer_properties.hpp>
#include <mln/style/layout_property.hpp>
#include <mln/style/paint_property.hpp>
#include <mln/style/properties.hpp>
#include <mln/shaders/attributes.hpp>
#include <mln/shaders/uniforms.hpp>
#include <mln/style/property_value.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/util/color.hpp>

#include <map>
#include <vector>

namespace mln {
namespace style {

using namespace conversion;

struct DataDrivenSingleFloatProperty : DataDrivenPaintProperty<float, attributes::width, uniforms::width> {
    static float defaultValue() { return 1.f; }
};

struct DataDrivenColorProperty : DataDrivenPaintProperty<mln::Color, attributes::color, uniforms::color> {
    static mln::Color defaultValue() { return mln::Color::black(); }
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
    const PropertyValue<mln::Color>& getColor() const;
    void setColor(const PropertyValue<mln::Color>& value);
    mln::Color _defaultColorValue = mln::Color::black();
    mln::Color _dataDrivenColorValue = mln::Color::black();
    PropertyValue<mln::Color> _dataDrivenColorProperty;
    void setCurrentColorValue(mln::Color value);

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

private:
    LayerTypeInfo _layerTypeInfo;
    std::string _layerProperties;
};

} // namespace style
} // namespace mln
