//
//  plugin_layer.cpp
//  App
//
//  Created by Malcolm Toon on 4/24/25.
//

#include <mbgl/plugins/plugin_layer.hpp>
#include <mbgl/plugins/plugin_layer_impl.hpp>
#include <iostream>

/*
//#include <mbgl/style/layers/heatmap_layer.hpp>
//#include <mbgl/style/layers/heatmap_layer_impl.hpp>
#include <mbgl/style/layer_observer.hpp>
//#include <mbgl/style/conversion/color_ramp_property_value.hpp>
//#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion/property_value.hpp>
//#include <mbgl/style/conversion/transition_options.hpp>
#include <mbgl/style/conversion/json.hpp>
//#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/traits.hpp>

#include <mapbox/eternal.hpp>
*/

/*
using namespace mbgl::style;
using namespace conversion;

namespace {

template <typename T>
constexpr uint8_t toUint8(T t) noexcept {
    return uint8_t(mbgl::underlying_type(t));
}


enum class Property : uint8_t {
    PluginProperty1,
};

constexpr const auto layerProperties = mapbox::eternal::hash_map<mapbox::eternal::string,
uint8_t>(
    {{"plugin-property1", toUint8(Property::PluginProperty1)}
                                                                                                   }
                                                                                                   );




StyleProperty getLayerProperty(const PluginLayer& layer, Property property) {

     switch (property) {
     case Property::HeatmapColor:
     return makeStyleProperty(layer.getHeatmapColor());
     case Property::HeatmapIntensity:
     return makeStyleProperty(layer.getHeatmapIntensity());
     case Property::HeatmapOpacity:
     return makeStyleProperty(layer.getHeatmapOpacity());
     case Property::HeatmapRadius:
     return makeStyleProperty(layer.getHeatmapRadius());
     case Property::HeatmapWeight:
     return makeStyleProperty(layer.getHeatmapWeight());
     case Property::HeatmapColorTransition:
     return makeStyleProperty(layer.getHeatmapColorTransition());
     case Property::HeatmapIntensityTransition:
     return makeStyleProperty(layer.getHeatmapIntensityTransition());
     case Property::HeatmapOpacityTransition:
     return makeStyleProperty(layer.getHeatmapOpacityTransition());
     case Property::HeatmapRadiusTransition:
     return makeStyleProperty(layer.getHeatmapRadiusTransition());
     case Property::HeatmapWeightTransition:
     return makeStyleProperty(layer.getHeatmapWeightTransition());
     }

    return {};
}

StyleProperty getLayerProperty(const PluginLayer& layer, const std::string& name) {
    const auto it = layerProperties.find(name.c_str());
    if (it == layerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<Property>(it->second));
}


}

*/

namespace mbgl {
namespace style {

// static
// const LayerTypeInfo* PluginLayer::Impl::staticTypeInfo() noexcept {
//    const static LayerTypeInfo typeInfo{.type = "heatmap",
//                                        .source = LayerTypeInfo::Source::Required,
//                                        .pass3d = LayerTypeInfo::Pass3D::Required,
//                                        .layout = LayerTypeInfo::Layout::NotRequired,
//                                        .fadingTiles = LayerTypeInfo::FadingTiles::NotRequired,
//                                        .crossTileIndex = LayerTypeInfo::CrossTileIndex::NotRequired,
//                                        .tileKind = LayerTypeInfo::TileKind::Geometry};
//    return &typeInfo;
//}
/*
void PluginLayer::setRenderFunction(OnRenderLayer renderFunction) {
    //    auto i = impl();
    //    auto i2 = mutableImpl();
    //    i.setRenderFunction(renderFunction);
    //    i2->setRenderFunction(renderFunction);
    _renderFunction = renderFunction;
}

void PluginLayer::setUpdateFunction(OnUpdateLayer updateFunction) {
    _updateFunction = updateFunction;
}
*/

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
/*
// Layout properties

constexpr const auto layerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"heatmap-color", toUint8(Property::HeatmapColor)},
     {"heatmap-intensity", toUint8(Property::HeatmapIntensity)},
     {"heatmap-opacity", toUint8(Property::HeatmapOpacity)},
     {"heatmap-radius", toUint8(Property::HeatmapRadius)},
     {"heatmap-weight", toUint8(Property::HeatmapWeight)},
     {"heatmap-color-transition", toUint8(Property::HeatmapColorTransition)},
     {"heatmap-intensity-transition", toUint8(Property::HeatmapIntensityTransition)},
     {"heatmap-opacity-transition", toUint8(Property::HeatmapOpacityTransition)},
     {"heatmap-radius-transition", toUint8(Property::HeatmapRadiusTransition)},
     {"heatmap-weight-transition", toUint8(Property::HeatmapWeightTransition)}});


*/

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
    // std::optional<Error> PluginLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    /*
     TODO: Implement this
        const auto it = layerProperties.find(name.c_str());
        if (it == layerProperties.end()) return Error{"layer doesn't support this property"};

        auto property = static_cast<Property>(it->second);

        return Error{"layer doesn't support this property"};
     */

    /*
    Property<


    auto property = static_cast<Property>(it->second);

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }
*/

    std::cout << "Property Name: " << name << "\n";

    // auto i = impl();
    auto i = (mbgl::style::PluginLayer::Impl*)baseImpl.get();

    PluginLayerProperty* property = i->_propertyManager.getProperty(name);
    if (property == nullptr) {
        property = new PluginLayerProperty();
        property->_propertyName = name;
        i->_propertyManager.addProperty(property);
    }
    /*
    auto property = i._propertyManager._properties[name];
    if (property == nullptr) {
        property = new PluginLayerProperty();
        property->_propertyName = name;
        i._propertyManager._properties[name] = property;
    }
    */
    // Hard coding for testing
    if (name == "scale") {
        //        auto tv = convert<Scale>(value, error, false, false)
        Error error;
        const auto& tv = convert<PropertyValue<Scale>>(value, error, false, false);
        //        const auto & tv = convert<Scale>(value, error, false, false);
        if (!tv) {
            return error;
        }
        const auto& tv2 = convert<PropertyValue<float>>(value, error, false, false);
        if (!tv2) {
            return error;
        }

        auto tv3 = tv2.value();

        property->setSingleFloat(tv3);

        //        property->setScale(*tv2);
        //        property->setScale2(*tv);
        //        property->setPropertyValue(value); // *tv);
    }
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
