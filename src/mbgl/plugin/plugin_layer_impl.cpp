#include <mbgl/plugin/plugin_layer_impl.hpp>

#include <iostream>

namespace mbgl {
namespace style {

PluginLayer::Impl::Impl(std::string layerID,
                        std::string sourceID,
                        LayerTypeInfo layerTypeInfo,
                        const std::string& layerProperties)
    : Layer::Impl(layerID, sourceID),
      _layerTypeInfo(layerTypeInfo),
      _layerProperties(layerProperties) {}

bool PluginLayer::Impl::hasLayoutDifference([[maybe_unused]] const Layer::Impl& other) const {
    return false;
}

// Return this property as json
std::string PluginLayerProperty::asJSON() {
    std::string tempResult;

    if (_propertyType == PropertyType::SingleFloat) {
        tempResult = "\"" + _propertyName + "\":" + std::to_string(_singleFloatValue);
    } else if (_propertyType == PropertyType::Color) {
        // In RGBA format
        tempResult = "\"" + _propertyName + "\":\"" + _dataDrivenColorValue.stringify() + "\"";
    }

    return tempResult;
}

void PluginLayerProperty::setPropertyValue([[maybe_unused]] const conversion::Convertible& value) {}

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
    if (_properties.contains(propertyName)) {
        return _properties[propertyName];
    }
    return nullptr;
}

void PluginLayerPropertyManager::addProperty(PluginLayerProperty* property) {
    _properties[property->_propertyName] = property;
}

std::string PluginLayerPropertyManager::propertiesAsJSON() {
    std::string tempResult = "{";

    bool firstItem = true;
    for (auto d : _properties) {
        if (!firstItem) {
            tempResult.append(", ");
        }
        firstItem = false;
        tempResult.append(d.second->asJSON());
    }
    tempResult.append("}");

    return tempResult;
}

std::vector<PluginLayerProperty*> PluginLayerPropertyManager::getProperties() {
    std::vector<PluginLayerProperty*> tempResult;
    tempResult.reserve(_properties.size());
    for (auto it = _properties.begin(); it != _properties.end(); ++it) {
        tempResult.push_back(it->second);
    }
    return tempResult;
}

const PropertyValue<mbgl::Color>& PluginLayerProperty::getColor() const {
    return _dataDrivenColorProperty;
}

void PluginLayerProperty::setColor(const PropertyValue<mbgl::Color>& value) {
    _dataDrivenColorProperty = std::move(value);
}

void PluginLayerProperty::setCurrentColorValue(mbgl::Color value) {
    _dataDrivenColorValue = value;
}

namespace conversion {} // namespace conversion

} // namespace style
} // namespace mbgl
