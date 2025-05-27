#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/plugin/plugin_layer.hpp>
#include <mbgl/style/conversion_impl.hpp>

// ---------------------------------------------------
// Properties stuff
#include <map>
#include <vector>
#include <mbgl/style/types.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/style/layout_property.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/shaders/attributes.hpp>
#include <mbgl/shaders/uniforms.hpp>

#include <mbgl/style/property_value.hpp>
#include <mbgl/style/conversion/property_value.hpp>

// Property types
#include <mbgl/util/color.hpp>


// ---------------------------------------------------

namespace mbgl {
namespace style {

using namespace conversion;

struct SingleFloatProperty : DataDrivenPaintProperty<float, attributes::width, uniforms::width> {
    static float defaultValue() { return 1.f; }
};

struct DataDrivenColorProperty : DataDrivenPaintProperty<mbgl::Color, attributes::color, uniforms::color> {
    static mbgl::Color defaultValue() { return mbgl::Color::black(); }
    static constexpr auto expressionType() { return expression::type::ColorType{}; };
    using EvaluatorType = DataDrivenPropertyEvaluator<Color, false>;
};

/*
 Unique PluginLayerProperty types for now
 SingleFloat:   DataDrivenPaintProperty<float>
 Color:         DataDrivenPaintProperty<Color>

 Float2:        DataDrivenPaintProperty<std::array<float, 2>>
 Alignment:     DataDrivenPaintProperty<AlignmentType>


 Unique property types (from Tim)
 : DataDrivenLayoutProperty<expression::Formatted>
 : DataDrivenLayoutProperty<expression::Image>
 : DataDrivenLayoutProperty<float>
 : DataDrivenLayoutProperty<LineJoinType>
 : DataDrivenLayoutProperty<Padding>
 : DataDrivenLayoutProperty<std::array<float, 2>>
 : DataDrivenLayoutProperty<std::vector<std::string>>
 : DataDrivenLayoutProperty<SymbolAnchorType>
 : DataDrivenLayoutProperty<TextJustifyType>
 : DataDrivenLayoutProperty<TextTransformType>
 : DataDrivenLayoutProperty<VariableAnchorOffsetCollection>
 : DataDrivenPaintProperty<Color, attributes::color, uniforms::color>
 : DataDrivenPaintProperty<Color, attributes::fill_color, uniforms::fill_color, true>
 : DataDrivenPaintProperty<Color, attributes::fill_color, uniforms::fill_color>
 : DataDrivenPaintProperty<Color, attributes::halo_color, uniforms::halo_color>
 : DataDrivenPaintProperty<Color, attributes::outline_color, uniforms::outline_color>
 : DataDrivenPaintProperty<Color, attributes::stroke_color, uniforms::stroke_color>
 : DataDrivenPaintProperty<float, attributes::base, uniforms::base>
 : DataDrivenPaintProperty<float, attributes::blur, uniforms::blur>
 : DataDrivenPaintProperty<float, attributes::floorwidth, uniforms::floorwidth>
 : DataDrivenPaintProperty<float, attributes::gapwidth, uniforms::gapwidth>
 : DataDrivenPaintProperty<float, attributes::halo_blur, uniforms::halo_blur>
 : DataDrivenPaintProperty<float, attributes::halo_width, uniforms::halo_width>
 : DataDrivenPaintProperty<float, attributes::height, uniforms::height>
 : DataDrivenPaintProperty<float, attributes::offset, uniforms::offset>
 : DataDrivenPaintProperty<float, attributes::opacity, uniforms::opacity>
 : DataDrivenPaintProperty<float, attributes::radius, uniforms::radius>
 : DataDrivenPaintProperty<float, attributes::stroke_opacity, uniforms::stroke_opacity>
 : DataDrivenPaintProperty<float, attributes::stroke_width, uniforms::stroke_width>
 : DataDrivenPaintProperty<float, attributes::weight, uniforms::weight>
 : DataDrivenPaintProperty<float, attributes::width, uniforms::width>
 : LayoutProperty<AlignmentType>
 : LayoutProperty<bool>
 : LayoutProperty<expression::Image>
 : LayoutProperty<float>
 : LayoutProperty<IconTextFitType>
 : LayoutProperty<LineCapType>
 : LayoutProperty<std::array<float, 4>>
 : LayoutProperty<std::vector<TextVariableAnchorType>>
 : LayoutProperty<std::vector<TextWritingModeType>>
 : LayoutProperty<SymbolPlacementType>
 : LayoutProperty<SymbolZOrderType>
 : PaintProperty<AlignmentType>
 : PaintProperty<bool>
 : PaintProperty<CirclePitchScaleType>
 : PaintProperty<Color>
 : PaintProperty<float>
 : PaintProperty<HillshadeIlluminationAnchorType>
 : PaintProperty<RasterResamplingType>
 : PaintProperty<Rotation>
 : PaintProperty<std::array<double, 3>>
 : PaintProperty<std::array<float, 2>>
 : PaintProperty<TranslateAnchorType>
 */

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
    // PluginPaintProperties::Transitionable paint;
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
    Impl(std::string layerID, std::string sourceID, LayerTypeInfo layerTypeInfo, const std::string& layerProperties
         //,const style::conversion::Convertible& layerProperties
    );

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
    // style::conversion::Convertible _layerProperties;

    // HeatmapPaintProperties::Transitionable paint;
};

} // namespace style
} // namespace mbgl
