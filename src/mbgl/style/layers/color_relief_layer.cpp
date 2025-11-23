// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/color_relief_layer.hpp>
#include <mbgl/style/layers/color_relief_layer_impl.hpp>
#include <mbgl/style/layer_observer.hpp>
#include <mbgl/style/conversion/color_ramp_property_value.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/style/conversion/transition_options.hpp>
#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/traits.hpp>

#include <mapbox/eternal.hpp>

namespace mbgl {
namespace style {


// static
const LayerTypeInfo* ColorReliefLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{.type="color-relief",
                                        .source=LayerTypeInfo::Source::Required,
                                        .pass3d=LayerTypeInfo::Pass3D::NotRequired,
                                        .layout=LayerTypeInfo::Layout::NotRequired,
                                        .fadingTiles=LayerTypeInfo::FadingTiles::NotRequired,
                                        .crossTileIndex=LayerTypeInfo::CrossTileIndex::NotRequired,
                                        .tileKind=LayerTypeInfo::TileKind::RasterDEM};
    return &typeInfo;
}

ColorReliefLayer::ColorReliefLayer(const std::string& layerID, const std::string& sourceID)
    : Layer(makeMutable<Impl>(layerID, sourceID)) {
}

ColorReliefLayer::ColorReliefLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

ColorReliefLayer::~ColorReliefLayer() {
    weakFactory.invalidateWeakPtrs();
}

const ColorReliefLayer::Impl& ColorReliefLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<ColorReliefLayer::Impl> ColorReliefLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> ColorReliefLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    impl_->paint = ColorReliefPaintProperties::Transitionable();
    return std::make_unique<ColorReliefLayer>(std::move(impl_));
}

void ColorReliefLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const {
}

// Layout properties


// Paint properties

ColorRampPropertyValue ColorReliefLayer::getDefaultColorReliefColor() {
    return {{}};
}

const ColorRampPropertyValue& ColorReliefLayer::getColorReliefColor() const {
    return impl().paint.template get<ColorReliefColor>().value;
}

void ColorReliefLayer::setColorReliefColor(const ColorRampPropertyValue& value) {
    if (value == getColorReliefColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<ColorReliefColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void ColorReliefLayer::setColorReliefColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<ColorReliefColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions ColorReliefLayer::getColorReliefColorTransition() const {
    return impl().paint.template get<ColorReliefColor>().options;
}

PropertyValue<float> ColorReliefLayer::getDefaultColorReliefOpacity() {
    return {1.f};
}

const PropertyValue<float>& ColorReliefLayer::getColorReliefOpacity() const {
    return impl().paint.template get<ColorReliefOpacity>().value;
}

void ColorReliefLayer::setColorReliefOpacity(const PropertyValue<float>& value) {
    if (value == getColorReliefOpacity())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<ColorReliefOpacity>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void ColorReliefLayer::setColorReliefOpacityTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<ColorReliefOpacity>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions ColorReliefLayer::getColorReliefOpacityTransition() const {
    return impl().paint.template get<ColorReliefOpacity>().options;
}

using namespace conversion;

namespace {

constexpr uint8_t kPaintPropertyCount = 4u;

enum class Property : uint8_t {
    ColorReliefColor,
    ColorReliefOpacity,
    ColorReliefColorTransition,
    ColorReliefOpacityTransition,
};

template <typename T>
constexpr uint8_t toUint8(T t) noexcept {
    return uint8_t(mbgl::underlying_type(t));
}

constexpr const auto layerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"color-relief-color", toUint8(Property::ColorReliefColor)},
     {"color-relief-opacity", toUint8(Property::ColorReliefOpacity)},
     {"color-relief-color-transition", toUint8(Property::ColorReliefColorTransition)},
     {"color-relief-opacity-transition", toUint8(Property::ColorReliefOpacityTransition)}});

StyleProperty getLayerProperty(const ColorReliefLayer& layer, Property property) {
    switch (property) {
        case Property::ColorReliefColor:
            return makeStyleProperty(layer.getColorReliefColor());
        case Property::ColorReliefOpacity:
            return makeStyleProperty(layer.getColorReliefOpacity());
        case Property::ColorReliefColorTransition:
            return makeStyleProperty(layer.getColorReliefColorTransition());
        case Property::ColorReliefOpacityTransition:
            return makeStyleProperty(layer.getColorReliefOpacityTransition());
    }
    return {};
}

StyleProperty getLayerProperty(const ColorReliefLayer& layer, const std::string& name) {
    const auto it = layerProperties.find(name.c_str());
    if (it == layerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<Property>(it->second));
}

} // namespace

Value ColorReliefLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : layerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<Property>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCount);
    }
    return result;
}

std::optional<Error> ColorReliefLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = layerProperties.find(name.c_str());
    if (it == layerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<Property>(it->second);

    if (property == Property::ColorReliefColor) {
        Error error;
        const auto& typedValue = convert<ColorRampPropertyValue>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setColorReliefColor(*typedValue);
        return std::nullopt;
    }
    if (property == Property::ColorReliefOpacity) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setColorReliefOpacity(*typedValue);
        return std::nullopt;
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == Property::ColorReliefColorTransition) {
        setColorReliefColorTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::ColorReliefOpacityTransition) {
        setColorReliefOpacityTransition(*transition);
        return std::nullopt;
    }

    return Error{"layer doesn't support this property"};
}

StyleProperty ColorReliefLayer::getProperty(const std::string& name) const {
    return getLayerProperty(*this, name);
}

Mutable<Layer::Impl> ColorReliefLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl

// clang-format on
