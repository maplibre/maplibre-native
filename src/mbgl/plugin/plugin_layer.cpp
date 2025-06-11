#include <mbgl/plugin/plugin_layer.hpp>
#include <mbgl/plugin/plugin_layer_impl.hpp>

#include <iostream>

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
    return result;
}

std::optional<conversion::Error> PluginLayer::setPropertyInternal(const std::string& name,
                                                                  const conversion::Convertible& value) {
    auto i = static_cast<const mbgl::style::PluginLayer::Impl*>(baseImpl.get());
    auto pm = i->_propertyManager;

    // The properties should be defined when the plugin layer is created
    PluginLayerProperty* property = pm.getProperty(name); // i->_propertyManager.getProperty(name);
    if (property == nullptr) {
        return Error{"layer doesn't support this property"};
    }

    Error error;
    if (property->_propertyType == PluginLayerProperty::PropertyType::SingleFloat) {
        const auto& tempValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!tempValue) {
            return error;
        }
        property->setSingleFloat(tempValue.value());
    } else if (property->_propertyType == PluginLayerProperty::PropertyType::Color) {
        const auto& tempValue = convert<PropertyValue<mbgl::Color>>(value, error, false, false);
        if (!tempValue) {
            return error;
        }
        property->setColor(tempValue.value());
    }

    return std::nullopt;
}

StyleProperty PluginLayer::getProperty([[maybe_unused]] const std::string& name) const {
    // TODO: Implement this via callback
    // return getLayerProperty(*this, name);
    return {};
}

Mutable<Layer::Impl> PluginLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl
