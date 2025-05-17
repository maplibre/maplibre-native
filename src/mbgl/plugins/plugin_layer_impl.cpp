//
//  plugin_layer_impl.cpp
//  App
//
//  Created by Malcolm Toon on 4/25/25.
//

#include "plugin_layer_impl.hpp"
#include <iostream>
#include "plugin_layer_debug.hpp"

namespace mbgl {
namespace style {

PluginLayer::Impl::Impl(std::string layerID,
                        std::string sourceID,
                        LayerTypeInfo layerTypeInfo,
                        const std::string& layerProperties
                        // ,const style::conversion::Convertible& layerProperties
                        )
    : Layer::Impl(layerID, sourceID),
      _layerTypeInfo(layerTypeInfo),
      _layerProperties(layerProperties)
//, _layerProperties(layerProperties)
{
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    std::cout << "Init\n";
#endif
    
   // auto d = new DataDrivenPropertyEvaluator<mbgl::Color, true>();
    
}

bool PluginLayer::Impl::hasLayoutDifference(const Layer::Impl& other) const {
    // TODO: Implement this
    return false;
    // assert(other.getTypeInfo() == getTypeInfo());
    //    const auto& impl = static_cast<const style::PluginLayer::Impl&>(other);
    //    return filter != impl.filter || visibility != impl.visibility ||
    //    paint.hasDataDrivenPropertyDifference(impl.paint);
}

// Return this property as json
std::string PluginLayerProperty::asJSON() {
    return "\"" + _propertyName + "\":" + std::to_string(_singleFloatValue);
}

void PluginLayerProperty::setPropertyValue(const conversion::Convertible& value) {
    // TODO: What goes here?
}

const PropertyValue<float>& PluginLayerProperty::getSingleFloat() const {
    return _singleFloatProperty;
}

void PluginLayerProperty::setSingleFloat(const PropertyValue<float>& value) {
    _singleFloatProperty = std::move(value);
}

void PluginLayerProperty::setCurrentSingleFloatValue(float value) {
    _singleFloatValue = value;
}

PluginLayerProperty* PluginLayerPropertyManager::getProperty(const std::string& propertyName) {
    if (_properties.count(propertyName) > 0) {
        return _properties[propertyName];
    }
    return nullptr;
}

void PluginLayerPropertyManager::addProperty(PluginLayerProperty* property) {
    _properties[property->_propertyName] = property;
}

// TODO: Possibly pass in the string
std::string PluginLayerPropertyManager::propertiesAsJSON() {
    std::string tempResult = "{";

    bool firstItem = true;
    for (auto d : _properties) {
        if (!firstItem) {
            tempResult.append(", ");
        }
        tempResult.append(d.second->asJSON());
    }
    tempResult.append("}");

    return tempResult;
}

#if INCLUDE_DATA_DRIVEN_COLOR_PROPERTY
// Color
const PropertyValue<DataDrivenColorProperty>& PluginLayerProperty::getColor() const {
    return _dataDrivenColorProperty;
}

void PluginLayerProperty::setColor(const PropertyValue<DataDrivenColorProperty>& value) {
    _dataDrivenColorProperty = std::move(value);
}

void PluginLayerProperty::setCurrentColorValue(Color value) {
    _dataDrivenColorValue = value;
}
#endif






namespace conversion {
/*
std::optional<Scale> Converter<Scale>::operator()(const Convertible& value, Error& error, bool, bool) const {
    return Scale();
}

std::optional<PropertyValue<Scale>> Converter<PropertyValue<Scale>>::operator()(const Convertible& value,
                                                                                Error& error,
                                                                                bool allowDataExpressions,
                                                                                bool convertTokens) const {
    using namespace mbgl::style::expression;
    if (isUndefined(value)) {
        return Scale();
    } else if (isExpression(value)) {
        ParsingContext ctx(type::Number);
        ParseResult expression = ctx.parseLayerPropertyExpression(value);
        if (!expression) {
            error.message = ctx.getCombinedErrors();
            return std::nullopt;
        }
        assert(*expression);
        if (!isFeatureConstant(**expression)) {
            error.message = "data expressions not supported";
            return std::nullopt;
        }
        //        if (!isZoomConstant(**expression)) {
        //            error.message = "zoom expressions not supported";
        //            return std::nullopt;
        //        }

        //        return ColorRampPropertyValue(std::move(*expression));
        return PropertyValue<Scale>(std::move(*expression));
    } else {
        error.message = "color ramp must be an expression";
        return std::nullopt;
    }

    return Scale();

}
 */

} // namespace conversion

} // namespace style
} // namespace mbgl
