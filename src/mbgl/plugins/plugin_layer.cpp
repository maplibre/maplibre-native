//
//  plugin_layer.cpp
//  App
//
//  Created by Malcolm Toon on 4/24/25.
//

#include <mbgl/plugins/plugin_layer.hpp>
#include <mbgl/plugins/plugin_layer_impl.hpp>
#include <iostream>
#include "plugin_layer_debug.hpp"


namespace mbgl {
namespace style {

PluginLayer::PluginLayer(const std::string& layerID,
                         const std::string& sourceID,
                         const style::LayerTypeInfo layerTypeInfo,
                         const std::string& layerProperties

                         //,const style::conversion::Convertible& layerProperties
                         )
    : Layer(makeMutable<Impl>(layerID, sourceID, layerTypeInfo, layerProperties)) {}

PluginLayer::PluginLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {}

PluginLayer::~PluginLayer() = default;

const PluginLayer::Impl& PluginLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<PluginLayer::Impl> PluginLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> PluginLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    return std::make_unique<PluginLayer>(std::move(impl_));
}

void PluginLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const {}

Value PluginLayer::serialize() const {
    auto result = Layer::serialize();
    /*
    // TODO: Implement this
    assert(result.getObject());

    for (const auto& property : layerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<Property>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCount);
    }
*/
    return result;
}

std::optional<conversion::Error> PluginLayer::setPropertyInternal(const std::string& name,
                                                                  const conversion::Convertible& value) {
    /*
     TODO: Implement this
        const auto it = layerProperties.find(name.c_str());
        if (it == layerProperties.end()) return Error{"layer doesn't support this property"};

        auto property = static_cast<Property>(it->second);

        return Error{"layer doesn't support this property"};
     */

#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    std::cout << "Property Name: " << name << "\n";
#endif

    auto i = (mbgl::style::PluginLayer::Impl*)baseImpl.get();

    PluginLayerProperty* property = i->_propertyManager.getProperty(name);
    if (property == nullptr) {
        property = new PluginLayerProperty();
        // TODO: This needs ot be passed in
        property->_propertyType = PluginLayerProperty::PropertyType::SingleFloat;
        property->_propertyName = name;
        i->_propertyManager.addProperty(property);
    }
    
    Error error;

    if (property->_propertyType == PluginLayerProperty::PropertyType::SingleFloat) {
        const auto& tempValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!tempValue) {
            return error;
        }
        property->setSingleFloat(tempValue.value());
    }
    
    /*
    // Hard coding for testing
    if (name == "scale") {
        //        auto tv = convert<Scale>(value, error, false, false)
        Error error;
//        const auto& tv = convert<PropertyValue<Scale>>(value, error, false, false);
//        //        const auto & tv = convert<Scale>(value, error, false, false);
//        if (!tv) {
//            return error;
//        }
        const auto& tv2 = convert<PropertyValue<float>>(value, error, false, false);
        if (!tv2) {
            return error;
        }

        //auto tv3 = tv2.value();

        property->setSingleFloat(tv2.value());

        //        property->setScale(*tv2);
        //        property->setScale2(*tv);
        //        property->setPropertyValue(value); // *tv);
    }
     */
    /*
    Error error;
    const auto& typedValue = convert<ColorRampPropertyValue>(value, error, false, false);
    if (!typedValue) {
        return error;
    }
*/
    // setHeatmapColor(*typedValue);

    return std::nullopt;

    // return Error{"Not implemented yet"};
}

StyleProperty PluginLayer::getProperty(const std::string& name) const {
    // TODO: Implement this via callback
    // return getLayerProperty(*this, name);
    return {};
}

Mutable<Layer::Impl> PluginLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl
