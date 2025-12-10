// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/hillshade_layer.hpp>
#include <mbgl/style/layers/hillshade_layer_impl.hpp>
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
const LayerTypeInfo* HillshadeLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{.type="hillshade",
                                        .source=LayerTypeInfo::Source::Required,
                                        .pass3d=LayerTypeInfo::Pass3D::Required,
                                        .layout=LayerTypeInfo::Layout::NotRequired,
                                        .fadingTiles=LayerTypeInfo::FadingTiles::NotRequired,
                                        .crossTileIndex=LayerTypeInfo::CrossTileIndex::NotRequired,
                                        .tileKind=LayerTypeInfo::TileKind::RasterDEM};
    return &typeInfo;
}

HillshadeLayer::HillshadeLayer(const std::string& layerID, const std::string& sourceID)
    : Layer(makeMutable<Impl>(layerID, sourceID)) {
}

HillshadeLayer::HillshadeLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

HillshadeLayer::~HillshadeLayer() {
    weakFactory.invalidateWeakPtrs();
}

const HillshadeLayer::Impl& HillshadeLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<HillshadeLayer::Impl> HillshadeLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> HillshadeLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    impl_->paint = HillshadePaintProperties::Transitionable();
    return std::make_unique<HillshadeLayer>(std::move(impl_));
}

void HillshadeLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const {
}

// Layout properties


// Paint properties

PropertyValue<Color> HillshadeLayer::getDefaultHillshadeAccentColor() {
    return {Color::black()};
}

const PropertyValue<Color>& HillshadeLayer::getHillshadeAccentColor() const {
    return impl().paint.template get<HillshadeAccentColor>().value;
}

void HillshadeLayer::setHillshadeAccentColor(const PropertyValue<Color>& value) {
    if (value == getHillshadeAccentColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeAccentColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void HillshadeLayer::setHillshadeAccentColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeAccentColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions HillshadeLayer::getHillshadeAccentColorTransition() const {
    return impl().paint.template get<HillshadeAccentColor>().options;
}

PropertyValue<float> HillshadeLayer::getDefaultHillshadeExaggeration() {
    return {0.5f};
}

const PropertyValue<float>& HillshadeLayer::getHillshadeExaggeration() const {
    return impl().paint.template get<HillshadeExaggeration>().value;
}

void HillshadeLayer::setHillshadeExaggeration(const PropertyValue<float>& value) {
    if (value == getHillshadeExaggeration())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeExaggeration>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void HillshadeLayer::setHillshadeExaggerationTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeExaggeration>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions HillshadeLayer::getHillshadeExaggerationTransition() const {
    return impl().paint.template get<HillshadeExaggeration>().options;
}

PropertyValue<std::vector<Color>> HillshadeLayer::getDefaultHillshadeHighlightColor() {
    return {{Color::white()}};
}

const PropertyValue<std::vector<Color>>& HillshadeLayer::getHillshadeHighlightColor() const {
    return impl().paint.template get<HillshadeHighlightColor>().value;
}

void HillshadeLayer::setHillshadeHighlightColor(const PropertyValue<std::vector<Color>>& value) {
    if (value == getHillshadeHighlightColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeHighlightColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void HillshadeLayer::setHillshadeHighlightColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeHighlightColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions HillshadeLayer::getHillshadeHighlightColorTransition() const {
    return impl().paint.template get<HillshadeHighlightColor>().options;
}

PropertyValue<std::vector<float>> HillshadeLayer::getDefaultHillshadeIlluminationAltitude() {
    return {{45.f}};
}

const PropertyValue<std::vector<float>>& HillshadeLayer::getHillshadeIlluminationAltitude() const {
    return impl().paint.template get<HillshadeIlluminationAltitude>().value;
}

void HillshadeLayer::setHillshadeIlluminationAltitude(const PropertyValue<std::vector<float>>& value) {
    if (value == getHillshadeIlluminationAltitude())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeIlluminationAltitude>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void HillshadeLayer::setHillshadeIlluminationAltitudeTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeIlluminationAltitude>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions HillshadeLayer::getHillshadeIlluminationAltitudeTransition() const {
    return impl().paint.template get<HillshadeIlluminationAltitude>().options;
}

PropertyValue<HillshadeIlluminationAnchorType> HillshadeLayer::getDefaultHillshadeIlluminationAnchor() {
    return {HillshadeIlluminationAnchorType::Viewport};
}

const PropertyValue<HillshadeIlluminationAnchorType>& HillshadeLayer::getHillshadeIlluminationAnchor() const {
    return impl().paint.template get<HillshadeIlluminationAnchor>().value;
}

void HillshadeLayer::setHillshadeIlluminationAnchor(const PropertyValue<HillshadeIlluminationAnchorType>& value) {
    if (value == getHillshadeIlluminationAnchor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeIlluminationAnchor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void HillshadeLayer::setHillshadeIlluminationAnchorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeIlluminationAnchor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions HillshadeLayer::getHillshadeIlluminationAnchorTransition() const {
    return impl().paint.template get<HillshadeIlluminationAnchor>().options;
}

PropertyValue<std::vector<float>> HillshadeLayer::getDefaultHillshadeIlluminationDirection() {
    return {{335.f}};
}

const PropertyValue<std::vector<float>>& HillshadeLayer::getHillshadeIlluminationDirection() const {
    return impl().paint.template get<HillshadeIlluminationDirection>().value;
}

void HillshadeLayer::setHillshadeIlluminationDirection(const PropertyValue<std::vector<float>>& value) {
    if (value == getHillshadeIlluminationDirection())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeIlluminationDirection>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void HillshadeLayer::setHillshadeIlluminationDirectionTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeIlluminationDirection>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions HillshadeLayer::getHillshadeIlluminationDirectionTransition() const {
    return impl().paint.template get<HillshadeIlluminationDirection>().options;
}

PropertyValue<HillshadeMethodType> HillshadeLayer::getDefaultHillshadeMethod() {
    return {HillshadeMethodType::Standard};
}

const PropertyValue<HillshadeMethodType>& HillshadeLayer::getHillshadeMethod() const {
    return impl().paint.template get<HillshadeMethod>().value;
}

void HillshadeLayer::setHillshadeMethod(const PropertyValue<HillshadeMethodType>& value) {
    if (value == getHillshadeMethod())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeMethod>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void HillshadeLayer::setHillshadeMethodTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeMethod>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions HillshadeLayer::getHillshadeMethodTransition() const {
    return impl().paint.template get<HillshadeMethod>().options;
}

PropertyValue<std::vector<Color>> HillshadeLayer::getDefaultHillshadeShadowColor() {
    return {{Color::black()}};
}

const PropertyValue<std::vector<Color>>& HillshadeLayer::getHillshadeShadowColor() const {
    return impl().paint.template get<HillshadeShadowColor>().value;
}

void HillshadeLayer::setHillshadeShadowColor(const PropertyValue<std::vector<Color>>& value) {
    if (value == getHillshadeShadowColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeShadowColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void HillshadeLayer::setHillshadeShadowColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<HillshadeShadowColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions HillshadeLayer::getHillshadeShadowColorTransition() const {
    return impl().paint.template get<HillshadeShadowColor>().options;
}

using namespace conversion;

namespace {

constexpr uint8_t kPaintPropertyCount = 16u;

enum class Property : uint8_t {
    HillshadeAccentColor,
    HillshadeExaggeration,
    HillshadeHighlightColor,
    HillshadeIlluminationAltitude,
    HillshadeIlluminationAnchor,
    HillshadeIlluminationDirection,
    HillshadeMethod,
    HillshadeShadowColor,
    HillshadeAccentColorTransition,
    HillshadeExaggerationTransition,
    HillshadeHighlightColorTransition,
    HillshadeIlluminationAltitudeTransition,
    HillshadeIlluminationAnchorTransition,
    HillshadeIlluminationDirectionTransition,
    HillshadeMethodTransition,
    HillshadeShadowColorTransition,
};

template <typename T>
constexpr uint8_t toUint8(T t) noexcept {
    return uint8_t(mbgl::underlying_type(t));
}

constexpr const auto layerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"hillshade-accent-color", toUint8(Property::HillshadeAccentColor)},
     {"hillshade-exaggeration", toUint8(Property::HillshadeExaggeration)},
     {"hillshade-highlight-color", toUint8(Property::HillshadeHighlightColor)},
     {"hillshade-illumination-altitude", toUint8(Property::HillshadeIlluminationAltitude)},
     {"hillshade-illumination-anchor", toUint8(Property::HillshadeIlluminationAnchor)},
     {"hillshade-illumination-direction", toUint8(Property::HillshadeIlluminationDirection)},
     {"hillshade-method", toUint8(Property::HillshadeMethod)},
     {"hillshade-shadow-color", toUint8(Property::HillshadeShadowColor)},
     {"hillshade-accent-color-transition", toUint8(Property::HillshadeAccentColorTransition)},
     {"hillshade-exaggeration-transition", toUint8(Property::HillshadeExaggerationTransition)},
     {"hillshade-highlight-color-transition", toUint8(Property::HillshadeHighlightColorTransition)},
     {"hillshade-illumination-altitude-transition", toUint8(Property::HillshadeIlluminationAltitudeTransition)},
     {"hillshade-illumination-anchor-transition", toUint8(Property::HillshadeIlluminationAnchorTransition)},
     {"hillshade-illumination-direction-transition", toUint8(Property::HillshadeIlluminationDirectionTransition)},
     {"hillshade-method-transition", toUint8(Property::HillshadeMethodTransition)},
     {"hillshade-shadow-color-transition", toUint8(Property::HillshadeShadowColorTransition)}});

StyleProperty getLayerProperty(const HillshadeLayer& layer, Property property) {
    switch (property) {
        case Property::HillshadeAccentColor:
            return makeStyleProperty(layer.getHillshadeAccentColor());
        case Property::HillshadeExaggeration:
            return makeStyleProperty(layer.getHillshadeExaggeration());
        case Property::HillshadeHighlightColor:
            return makeStyleProperty(layer.getHillshadeHighlightColor());
        case Property::HillshadeIlluminationAltitude:
            return makeStyleProperty(layer.getHillshadeIlluminationAltitude());
        case Property::HillshadeIlluminationAnchor:
            return makeStyleProperty(layer.getHillshadeIlluminationAnchor());
        case Property::HillshadeIlluminationDirection:
            return makeStyleProperty(layer.getHillshadeIlluminationDirection());
        case Property::HillshadeMethod:
            return makeStyleProperty(layer.getHillshadeMethod());
        case Property::HillshadeShadowColor:
            return makeStyleProperty(layer.getHillshadeShadowColor());
        case Property::HillshadeAccentColorTransition:
            return makeStyleProperty(layer.getHillshadeAccentColorTransition());
        case Property::HillshadeExaggerationTransition:
            return makeStyleProperty(layer.getHillshadeExaggerationTransition());
        case Property::HillshadeHighlightColorTransition:
            return makeStyleProperty(layer.getHillshadeHighlightColorTransition());
        case Property::HillshadeIlluminationAltitudeTransition:
            return makeStyleProperty(layer.getHillshadeIlluminationAltitudeTransition());
        case Property::HillshadeIlluminationAnchorTransition:
            return makeStyleProperty(layer.getHillshadeIlluminationAnchorTransition());
        case Property::HillshadeIlluminationDirectionTransition:
            return makeStyleProperty(layer.getHillshadeIlluminationDirectionTransition());
        case Property::HillshadeMethodTransition:
            return makeStyleProperty(layer.getHillshadeMethodTransition());
        case Property::HillshadeShadowColorTransition:
            return makeStyleProperty(layer.getHillshadeShadowColorTransition());
    }
    return {};
}

StyleProperty getLayerProperty(const HillshadeLayer& layer, const std::string& name) {
    const auto it = layerProperties.find(name.c_str());
    if (it == layerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<Property>(it->second));
}

} // namespace

Value HillshadeLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : layerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<Property>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCount);
    }
    return result;
}

std::optional<Error> HillshadeLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = layerProperties.find(name.c_str());
    if (it == layerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<Property>(it->second);

    if (property == Property::HillshadeAccentColor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<Color>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setHillshadeAccentColor(*typedValue);
        return std::nullopt;
    }
    if (property == Property::HillshadeExaggeration) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setHillshadeExaggeration(*typedValue);
        return std::nullopt;
    }
    if (property == Property::HillshadeHighlightColor || property == Property::HillshadeShadowColor) {
        Error error;
        // These properties accept either a single color or an array
        // Convert single colors to PropertyValue directly
        if (!isArray(value) && !isObject(value) && !isUndefined(value)) {
            auto color = convert<Color>(value, error);
            if (color) {
                // Wrap single color in a vector
                std::vector<Color> vec = {*color};
                if (property == Property::HillshadeHighlightColor) {
                    setHillshadeHighlightColor(PropertyValue<std::vector<Color>>(vec));
                    return std::nullopt;
                }
                if (property == Property::HillshadeShadowColor) {
                    setHillshadeShadowColor(PropertyValue<std::vector<Color>>(vec));
                    return std::nullopt;
                }
            }
        }

        const auto& typedValue = convert<PropertyValue<std::vector<Color>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == Property::HillshadeHighlightColor) {
            setHillshadeHighlightColor(*typedValue);
            return std::nullopt;
        }

        if (property == Property::HillshadeShadowColor) {
            setHillshadeShadowColor(*typedValue);
            return std::nullopt;
        }
    }
    if (property == Property::HillshadeIlluminationAltitude || property == Property::HillshadeIlluminationDirection) {
        Error error;
        // These properties accept either a single number or an array
        // Convert single numbers to PropertyValue directly
        if (!isArray(value) && !isObject(value) && !isUndefined(value)) {
            auto num = toNumber(value);
            if (num) {
                // Wrap single number in a vector
                std::vector<float> vec = {static_cast<float>(*num)};
                if (property == Property::HillshadeIlluminationAltitude) {
                    setHillshadeIlluminationAltitude(PropertyValue<std::vector<float>>(vec));
                    return std::nullopt;
                }
                if (property == Property::HillshadeIlluminationDirection) {
                    setHillshadeIlluminationDirection(PropertyValue<std::vector<float>>(vec));
                    return std::nullopt;
                }
            }
        }

        const auto& typedValue = convert<PropertyValue<std::vector<float>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == Property::HillshadeIlluminationAltitude) {
            setHillshadeIlluminationAltitude(*typedValue);
            return std::nullopt;
        }

        if (property == Property::HillshadeIlluminationDirection) {
            setHillshadeIlluminationDirection(*typedValue);
            return std::nullopt;
        }
    }
    if (property == Property::HillshadeIlluminationAnchor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<HillshadeIlluminationAnchorType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setHillshadeIlluminationAnchor(*typedValue);
        return std::nullopt;
    }
    if (property == Property::HillshadeMethod) {
        Error error;
        const auto& typedValue = convert<PropertyValue<HillshadeMethodType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setHillshadeMethod(*typedValue);
        return std::nullopt;
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == Property::HillshadeAccentColorTransition) {
        setHillshadeAccentColorTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::HillshadeExaggerationTransition) {
        setHillshadeExaggerationTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::HillshadeHighlightColorTransition) {
        setHillshadeHighlightColorTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::HillshadeIlluminationAltitudeTransition) {
        setHillshadeIlluminationAltitudeTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::HillshadeIlluminationAnchorTransition) {
        setHillshadeIlluminationAnchorTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::HillshadeIlluminationDirectionTransition) {
        setHillshadeIlluminationDirectionTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::HillshadeMethodTransition) {
        setHillshadeMethodTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::HillshadeShadowColorTransition) {
        setHillshadeShadowColorTransition(*transition);
        return std::nullopt;
    }

    return Error{"layer doesn't support this property"};
}

StyleProperty HillshadeLayer::getProperty(const std::string& name) const {
    return getLayerProperty(*this, name);
}

Mutable<Layer::Impl> HillshadeLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl

// clang-format on
