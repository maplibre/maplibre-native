//
//  plugin_layer_impl.cpp
//  App
//
//  Created by Malcolm Toon on 4/25/25.
//

#include "plugin_layer_impl.hpp"
#include <iostream>

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
    std::cout << "Init\n";
}

bool PluginLayer::Impl::hasLayoutDifference(const Layer::Impl& other) const {
    // TODO: Implement this
    return false;
    // assert(other.getTypeInfo() == getTypeInfo());
    //    const auto& impl = static_cast<const style::PluginLayer::Impl&>(other);
    //    return filter != impl.filter || visibility != impl.visibility ||
    //    paint.hasDataDrivenPropertyDifference(impl.paint);
}

void PluginLayerProperty::setPropertyValue(const conversion::Convertible& value) {
    // TODO: What goes here?
}
/*
void PluginLayerProperty::setTypedPropertyValue(const PropertyValue<Scale>& value) {
    if (value == getHeatmapColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<HeatmapColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
*/

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

const PropertyValue<float>& PluginLayerProperty::getScale() const {
    return paint.template get<Scale>().value;

    // return impl().paint.template get<LineWidth>().value;
}

void PluginLayerProperty::setScale(const PropertyValue<float>& value) {
    if (value == getScale()) return;

    paint.template get<Scale>().value = std::move(value);
    /*
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineWidth>().value = value;
    impl_->paint.template get<LineFloorWidth>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
     */
}

void PluginLayerProperty::setScaleTransition(const TransitionOptions& options) {
    paint.template get<Scale>().options = options;

    //    auto impl_ = mutableImpl();
    //    impl_->paint.template get<LineWidth>().options = options;
    //    baseImpl = std::move(impl_);
}

TransitionOptions PluginLayerProperty::getScaleTransition() const {
    return paint.template get<Scale>().options;

    // return impl().paint.template get<LineWidth>().options;
}

void PluginLayerProperty::setScale2(const PropertyValue<Scale>& value) {
    // paint.template get<Scale>().value = value;

    /*
    void FillLayer::setFillColor(const PropertyValue<Color>& value) {
        if (value == getFillColor())
            return;
        auto impl_ = mutableImpl();
        impl_->paint.template get<FillColor>().value = value;
        baseImpl = std::move(impl_);
        observer->onLayerChanged(*this);
    }
*/

    // paint.template get<Scale>() = std::move(value);
}

namespace conversion {

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

    /*
     using namespace mbgl::style::expression;
     if (isUndefined(value)) {
     return ColorRampPropertyValue();
     } else if (isExpression(value)) {
     ParsingContext ctx(type::Color);
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
     if (!isZoomConstant(**expression)) {
     error.message = "zoom expressions not supported";
     return std::nullopt;
     }
     return ColorRampPropertyValue(std::move(*expression));
     } else {
     error.message = "color ramp must be an expression";
     return std::nullopt;
     }
     */
}
} // namespace conversion

} // namespace style
} // namespace mbgl
