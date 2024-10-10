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
#include <mbgl/util/enum.hpp>

#include <mapbox/eternal.hpp>

namespace mbgl {
namespace style {


// static
const LayerTypeInfo* HillshadeLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{"hillshade",
                                        LayerTypeInfo::Source::Required,
                                        LayerTypeInfo::Pass3D::Required,
                                        LayerTypeInfo::Layout::NotRequired,
                                        LayerTypeInfo::FadingTiles::NotRequired,
                                        LayerTypeInfo::CrossTileIndex::NotRequired,
                                        LayerTypeInfo::TileKind::RasterDEM};
    return &typeInfo;
}


HillshadeLayer::HillshadeLayer(const std::string& layerID, const std::string& sourceID)
    : Layer(makeMutable<Impl>(layerID, sourceID)) {
}

HillshadeLayer::HillshadeLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

HillshadeLayer::~HillshadeLayer() = default;

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

PropertyValue<Color> HillshadeLayer::getDefaultHillshadeHighlightColor() {
    return {Color::white()};
}

const PropertyValue<Color>& HillshadeLayer::getHillshadeHighlightColor() const {
    return impl().paint.template get<HillshadeHighlightColor>().value;
}

void HillshadeLayer::setHillshadeHighlightColor(const PropertyValue<Color>& value) {
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

PropertyValue<float> HillshadeLayer::getDefaultHillshadeIlluminationDirection() {
    return {335.f};
}

const PropertyValue<float>& HillshadeLayer::getHillshadeIlluminationDirection() const {
    return impl().paint.template get<HillshadeIlluminationDirection>().value;
}

void HillshadeLayer::setHillshadeIlluminationDirection(const PropertyValue<float>& value) {
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

PropertyValue<Color> HillshadeLayer::getDefaultHillshadeShadowColor() {
    return {Color::black()};
}

const PropertyValue<Color>& HillshadeLayer::getHillshadeShadowColor() const {
    return impl().paint.template get<HillshadeShadowColor>().value;
}

void HillshadeLayer::setHillshadeShadowColor(const PropertyValue<Color>& value) {
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


constexpr uint8_t kPaintPropertyCountHillshade = 12u;


enum class HillshadeProperty : uint8_t {
    HillshadeAccentColor,
    HillshadeExaggeration,
    HillshadeHighlightColor,
    HillshadeIlluminationAnchor,
    HillshadeIlluminationDirection,
    HillshadeShadowColor,
    HillshadeAccentColorTransition,
    HillshadeExaggerationTransition,
    HillshadeHighlightColorTransition,
    HillshadeIlluminationAnchorTransition,
    HillshadeIlluminationDirectionTransition,
    HillshadeShadowColorTransition,
};


constexpr const auto hillshadeLayerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"hillshade-accent-color", toUint8(HillshadeProperty::HillshadeAccentColor)},
     {"hillshade-exaggeration", toUint8(HillshadeProperty::HillshadeExaggeration)},
     {"hillshade-highlight-color", toUint8(HillshadeProperty::HillshadeHighlightColor)},
     {"hillshade-illumination-anchor", toUint8(HillshadeProperty::HillshadeIlluminationAnchor)},
     {"hillshade-illumination-direction", toUint8(HillshadeProperty::HillshadeIlluminationDirection)},
     {"hillshade-shadow-color", toUint8(HillshadeProperty::HillshadeShadowColor)},
     {"hillshade-accent-color-transition", toUint8(HillshadeProperty::HillshadeAccentColorTransition)},
     {"hillshade-exaggeration-transition", toUint8(HillshadeProperty::HillshadeExaggerationTransition)},
     {"hillshade-highlight-color-transition", toUint8(HillshadeProperty::HillshadeHighlightColorTransition)},
     {"hillshade-illumination-anchor-transition", toUint8(HillshadeProperty::HillshadeIlluminationAnchorTransition)},
     {"hillshade-illumination-direction-transition", toUint8(HillshadeProperty::HillshadeIlluminationDirectionTransition)},
     {"hillshade-shadow-color-transition", toUint8(HillshadeProperty::HillshadeShadowColorTransition)}});

StyleProperty getLayerProperty(const HillshadeLayer& layer, HillshadeProperty property) {
    switch (property) {
        case HillshadeProperty::HillshadeAccentColor:
            return makeStyleProperty(layer.getHillshadeAccentColor());
        case HillshadeProperty::HillshadeExaggeration:
            return makeStyleProperty(layer.getHillshadeExaggeration());
        case HillshadeProperty::HillshadeHighlightColor:
            return makeStyleProperty(layer.getHillshadeHighlightColor());
        case HillshadeProperty::HillshadeIlluminationAnchor:
            return makeStyleProperty(layer.getHillshadeIlluminationAnchor());
        case HillshadeProperty::HillshadeIlluminationDirection:
            return makeStyleProperty(layer.getHillshadeIlluminationDirection());
        case HillshadeProperty::HillshadeShadowColor:
            return makeStyleProperty(layer.getHillshadeShadowColor());
        case HillshadeProperty::HillshadeAccentColorTransition:
            return makeStyleProperty(layer.getHillshadeAccentColorTransition());
        case HillshadeProperty::HillshadeExaggerationTransition:
            return makeStyleProperty(layer.getHillshadeExaggerationTransition());
        case HillshadeProperty::HillshadeHighlightColorTransition:
            return makeStyleProperty(layer.getHillshadeHighlightColorTransition());
        case HillshadeProperty::HillshadeIlluminationAnchorTransition:
            return makeStyleProperty(layer.getHillshadeIlluminationAnchorTransition());
        case HillshadeProperty::HillshadeIlluminationDirectionTransition:
            return makeStyleProperty(layer.getHillshadeIlluminationDirectionTransition());
        case HillshadeProperty::HillshadeShadowColorTransition:
            return makeStyleProperty(layer.getHillshadeShadowColorTransition());
    }
    return {};
}

StyleProperty getLayerProperty(const HillshadeLayer& layer, const std::string& name) {
    const auto it = hillshadeLayerProperties.find(name.c_str());
    if (it == hillshadeLayerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<HillshadeProperty>(it->second));
}

} // namespace

Value HillshadeLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : hillshadeLayerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<HillshadeProperty>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCountHillshade);
    }
    return result;
}

std::optional<Error> HillshadeLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = hillshadeLayerProperties.find(name.c_str());
    if (it == hillshadeLayerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<HillshadeProperty>(it->second);

    if (property == HillshadeProperty::HillshadeAccentColor || property == HillshadeProperty::HillshadeHighlightColor ||
        property == HillshadeProperty::HillshadeShadowColor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<Color>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == HillshadeProperty::HillshadeAccentColor) {
            setHillshadeAccentColor(*typedValue);
            return std::nullopt;
        }

        if (property == HillshadeProperty::HillshadeHighlightColor) {
            setHillshadeHighlightColor(*typedValue);
            return std::nullopt;
        }

        if (property == HillshadeProperty::HillshadeShadowColor) {
            setHillshadeShadowColor(*typedValue);
            return std::nullopt;
        }
    }
    if (property == HillshadeProperty::HillshadeExaggeration ||
        property == HillshadeProperty::HillshadeIlluminationDirection) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == HillshadeProperty::HillshadeExaggeration) {
            setHillshadeExaggeration(*typedValue);
            return std::nullopt;
        }

        if (property == HillshadeProperty::HillshadeIlluminationDirection) {
            setHillshadeIlluminationDirection(*typedValue);
            return std::nullopt;
        }
    }
    if (property == HillshadeProperty::HillshadeIlluminationAnchor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<HillshadeIlluminationAnchorType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setHillshadeIlluminationAnchor(*typedValue);
        return std::nullopt;
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == HillshadeProperty::HillshadeAccentColorTransition) {
        setHillshadeAccentColorTransition(*transition);
        return std::nullopt;
    }

    if (property == HillshadeProperty::HillshadeExaggerationTransition) {
        setHillshadeExaggerationTransition(*transition);
        return std::nullopt;
    }

    if (property == HillshadeProperty::HillshadeHighlightColorTransition) {
        setHillshadeHighlightColorTransition(*transition);
        return std::nullopt;
    }

    if (property == HillshadeProperty::HillshadeIlluminationAnchorTransition) {
        setHillshadeIlluminationAnchorTransition(*transition);
        return std::nullopt;
    }

    if (property == HillshadeProperty::HillshadeIlluminationDirectionTransition) {
        setHillshadeIlluminationDirectionTransition(*transition);
        return std::nullopt;
    }

    if (property == HillshadeProperty::HillshadeShadowColorTransition) {
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
