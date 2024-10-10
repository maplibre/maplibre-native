// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/layers/circle_layer_impl.hpp>
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
const LayerTypeInfo* CircleLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{"circle",
                                        LayerTypeInfo::Source::Required,
                                        LayerTypeInfo::Pass3D::NotRequired,
                                        LayerTypeInfo::Layout::Required,
                                        LayerTypeInfo::FadingTiles::NotRequired,
                                        LayerTypeInfo::CrossTileIndex::NotRequired,
                                        LayerTypeInfo::TileKind::Geometry};
    return &typeInfo;
}


CircleLayer::CircleLayer(const std::string& layerID, const std::string& sourceID)
    : Layer(makeMutable<Impl>(layerID, sourceID)) {
}

CircleLayer::CircleLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

CircleLayer::~CircleLayer() = default;

const CircleLayer::Impl& CircleLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<CircleLayer::Impl> CircleLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> CircleLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    impl_->paint = CirclePaintProperties::Transitionable();
    return std::make_unique<CircleLayer>(std::move(impl_));
}

void CircleLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>& writer) const {
    layout.stringify(writer);
}

// Layout properties

PropertyValue<float> CircleLayer::getDefaultCircleSortKey() {
    return CircleSortKey::defaultValue();
}

const PropertyValue<float>& CircleLayer::getCircleSortKey() const {
    return impl().layout.get<CircleSortKey>();
}

void CircleLayer::setCircleSortKey(const PropertyValue<float>& value) {
    if (value == getCircleSortKey()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<CircleSortKey>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

// Paint properties

PropertyValue<float> CircleLayer::getDefaultCircleBlur() {
    return {0.f};
}

const PropertyValue<float>& CircleLayer::getCircleBlur() const {
    return impl().paint.template get<CircleBlur>().value;
}

void CircleLayer::setCircleBlur(const PropertyValue<float>& value) {
    if (value == getCircleBlur())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleBlur>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCircleBlurTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleBlur>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCircleBlurTransition() const {
    return impl().paint.template get<CircleBlur>().options;
}

PropertyValue<Color> CircleLayer::getDefaultCircleColor() {
    return {Color::black()};
}

const PropertyValue<Color>& CircleLayer::getCircleColor() const {
    return impl().paint.template get<CircleColor>().value;
}

void CircleLayer::setCircleColor(const PropertyValue<Color>& value) {
    if (value == getCircleColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCircleColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCircleColorTransition() const {
    return impl().paint.template get<CircleColor>().options;
}

PropertyValue<float> CircleLayer::getDefaultCircleOpacity() {
    return {1.f};
}

const PropertyValue<float>& CircleLayer::getCircleOpacity() const {
    return impl().paint.template get<CircleOpacity>().value;
}

void CircleLayer::setCircleOpacity(const PropertyValue<float>& value) {
    if (value == getCircleOpacity())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleOpacity>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCircleOpacityTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleOpacity>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCircleOpacityTransition() const {
    return impl().paint.template get<CircleOpacity>().options;
}

PropertyValue<AlignmentType> CircleLayer::getDefaultCirclePitchAlignment() {
    return {AlignmentType::Viewport};
}

const PropertyValue<AlignmentType>& CircleLayer::getCirclePitchAlignment() const {
    return impl().paint.template get<CirclePitchAlignment>().value;
}

void CircleLayer::setCirclePitchAlignment(const PropertyValue<AlignmentType>& value) {
    if (value == getCirclePitchAlignment())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CirclePitchAlignment>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCirclePitchAlignmentTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CirclePitchAlignment>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCirclePitchAlignmentTransition() const {
    return impl().paint.template get<CirclePitchAlignment>().options;
}

PropertyValue<CirclePitchScaleType> CircleLayer::getDefaultCirclePitchScale() {
    return {CirclePitchScaleType::Map};
}

const PropertyValue<CirclePitchScaleType>& CircleLayer::getCirclePitchScale() const {
    return impl().paint.template get<CirclePitchScale>().value;
}

void CircleLayer::setCirclePitchScale(const PropertyValue<CirclePitchScaleType>& value) {
    if (value == getCirclePitchScale())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CirclePitchScale>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCirclePitchScaleTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CirclePitchScale>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCirclePitchScaleTransition() const {
    return impl().paint.template get<CirclePitchScale>().options;
}

PropertyValue<float> CircleLayer::getDefaultCircleRadius() {
    return {5.f};
}

const PropertyValue<float>& CircleLayer::getCircleRadius() const {
    return impl().paint.template get<CircleRadius>().value;
}

void CircleLayer::setCircleRadius(const PropertyValue<float>& value) {
    if (value == getCircleRadius())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleRadius>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCircleRadiusTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleRadius>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCircleRadiusTransition() const {
    return impl().paint.template get<CircleRadius>().options;
}

PropertyValue<Color> CircleLayer::getDefaultCircleStrokeColor() {
    return {Color::black()};
}

const PropertyValue<Color>& CircleLayer::getCircleStrokeColor() const {
    return impl().paint.template get<CircleStrokeColor>().value;
}

void CircleLayer::setCircleStrokeColor(const PropertyValue<Color>& value) {
    if (value == getCircleStrokeColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleStrokeColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCircleStrokeColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleStrokeColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCircleStrokeColorTransition() const {
    return impl().paint.template get<CircleStrokeColor>().options;
}

PropertyValue<float> CircleLayer::getDefaultCircleStrokeOpacity() {
    return {1.f};
}

const PropertyValue<float>& CircleLayer::getCircleStrokeOpacity() const {
    return impl().paint.template get<CircleStrokeOpacity>().value;
}

void CircleLayer::setCircleStrokeOpacity(const PropertyValue<float>& value) {
    if (value == getCircleStrokeOpacity())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleStrokeOpacity>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCircleStrokeOpacityTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleStrokeOpacity>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCircleStrokeOpacityTransition() const {
    return impl().paint.template get<CircleStrokeOpacity>().options;
}

PropertyValue<float> CircleLayer::getDefaultCircleStrokeWidth() {
    return {0.f};
}

const PropertyValue<float>& CircleLayer::getCircleStrokeWidth() const {
    return impl().paint.template get<CircleStrokeWidth>().value;
}

void CircleLayer::setCircleStrokeWidth(const PropertyValue<float>& value) {
    if (value == getCircleStrokeWidth())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleStrokeWidth>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCircleStrokeWidthTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleStrokeWidth>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCircleStrokeWidthTransition() const {
    return impl().paint.template get<CircleStrokeWidth>().options;
}

PropertyValue<std::array<float, 2>> CircleLayer::getDefaultCircleTranslate() {
    return {{{0.f, 0.f}}};
}

const PropertyValue<std::array<float, 2>>& CircleLayer::getCircleTranslate() const {
    return impl().paint.template get<CircleTranslate>().value;
}

void CircleLayer::setCircleTranslate(const PropertyValue<std::array<float, 2>>& value) {
    if (value == getCircleTranslate())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleTranslate>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCircleTranslateTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleTranslate>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCircleTranslateTransition() const {
    return impl().paint.template get<CircleTranslate>().options;
}

PropertyValue<TranslateAnchorType> CircleLayer::getDefaultCircleTranslateAnchor() {
    return {TranslateAnchorType::Map};
}

const PropertyValue<TranslateAnchorType>& CircleLayer::getCircleTranslateAnchor() const {
    return impl().paint.template get<CircleTranslateAnchor>().value;
}

void CircleLayer::setCircleTranslateAnchor(const PropertyValue<TranslateAnchorType>& value) {
    if (value == getCircleTranslateAnchor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleTranslateAnchor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void CircleLayer::setCircleTranslateAnchorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<CircleTranslateAnchor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions CircleLayer::getCircleTranslateAnchorTransition() const {
    return impl().paint.template get<CircleTranslateAnchor>().options;
}

using namespace conversion;

namespace {


constexpr uint8_t kPaintPropertyCountCircle = 22u;


enum class CircleProperty : uint8_t {
    CircleBlur,
    CircleColor,
    CircleOpacity,
    CirclePitchAlignment,
    CirclePitchScale,
    CircleRadius,
    CircleStrokeColor,
    CircleStrokeOpacity,
    CircleStrokeWidth,
    CircleTranslate,
    CircleTranslateAnchor,
    CircleBlurTransition,
    CircleColorTransition,
    CircleOpacityTransition,
    CirclePitchAlignmentTransition,
    CirclePitchScaleTransition,
    CircleRadiusTransition,
    CircleStrokeColorTransition,
    CircleStrokeOpacityTransition,
    CircleStrokeWidthTransition,
    CircleTranslateTransition,
    CircleTranslateAnchorTransition,
    CircleSortKey = kPaintPropertyCountCircle,
};


constexpr const auto circleLayerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"circle-blur", toUint8(CircleProperty::CircleBlur)},
     {"circle-color", toUint8(CircleProperty::CircleColor)},
     {"circle-opacity", toUint8(CircleProperty::CircleOpacity)},
     {"circle-pitch-alignment", toUint8(CircleProperty::CirclePitchAlignment)},
     {"circle-pitch-scale", toUint8(CircleProperty::CirclePitchScale)},
     {"circle-radius", toUint8(CircleProperty::CircleRadius)},
     {"circle-stroke-color", toUint8(CircleProperty::CircleStrokeColor)},
     {"circle-stroke-opacity", toUint8(CircleProperty::CircleStrokeOpacity)},
     {"circle-stroke-width", toUint8(CircleProperty::CircleStrokeWidth)},
     {"circle-translate", toUint8(CircleProperty::CircleTranslate)},
     {"circle-translate-anchor", toUint8(CircleProperty::CircleTranslateAnchor)},
     {"circle-blur-transition", toUint8(CircleProperty::CircleBlurTransition)},
     {"circle-color-transition", toUint8(CircleProperty::CircleColorTransition)},
     {"circle-opacity-transition", toUint8(CircleProperty::CircleOpacityTransition)},
     {"circle-pitch-alignment-transition", toUint8(CircleProperty::CirclePitchAlignmentTransition)},
     {"circle-pitch-scale-transition", toUint8(CircleProperty::CirclePitchScaleTransition)},
     {"circle-radius-transition", toUint8(CircleProperty::CircleRadiusTransition)},
     {"circle-stroke-color-transition", toUint8(CircleProperty::CircleStrokeColorTransition)},
     {"circle-stroke-opacity-transition", toUint8(CircleProperty::CircleStrokeOpacityTransition)},
     {"circle-stroke-width-transition", toUint8(CircleProperty::CircleStrokeWidthTransition)},
     {"circle-translate-transition", toUint8(CircleProperty::CircleTranslateTransition)},
     {"circle-translate-anchor-transition", toUint8(CircleProperty::CircleTranslateAnchorTransition)},
     {"circle-sort-key", toUint8(CircleProperty::CircleSortKey)}});

StyleProperty getLayerProperty(const CircleLayer& layer, CircleProperty property) {
    switch (property) {
        case CircleProperty::CircleBlur:
            return makeStyleProperty(layer.getCircleBlur());
        case CircleProperty::CircleColor:
            return makeStyleProperty(layer.getCircleColor());
        case CircleProperty::CircleOpacity:
            return makeStyleProperty(layer.getCircleOpacity());
        case CircleProperty::CirclePitchAlignment:
            return makeStyleProperty(layer.getCirclePitchAlignment());
        case CircleProperty::CirclePitchScale:
            return makeStyleProperty(layer.getCirclePitchScale());
        case CircleProperty::CircleRadius:
            return makeStyleProperty(layer.getCircleRadius());
        case CircleProperty::CircleStrokeColor:
            return makeStyleProperty(layer.getCircleStrokeColor());
        case CircleProperty::CircleStrokeOpacity:
            return makeStyleProperty(layer.getCircleStrokeOpacity());
        case CircleProperty::CircleStrokeWidth:
            return makeStyleProperty(layer.getCircleStrokeWidth());
        case CircleProperty::CircleTranslate:
            return makeStyleProperty(layer.getCircleTranslate());
        case CircleProperty::CircleTranslateAnchor:
            return makeStyleProperty(layer.getCircleTranslateAnchor());
        case CircleProperty::CircleBlurTransition:
            return makeStyleProperty(layer.getCircleBlurTransition());
        case CircleProperty::CircleColorTransition:
            return makeStyleProperty(layer.getCircleColorTransition());
        case CircleProperty::CircleOpacityTransition:
            return makeStyleProperty(layer.getCircleOpacityTransition());
        case CircleProperty::CirclePitchAlignmentTransition:
            return makeStyleProperty(layer.getCirclePitchAlignmentTransition());
        case CircleProperty::CirclePitchScaleTransition:
            return makeStyleProperty(layer.getCirclePitchScaleTransition());
        case CircleProperty::CircleRadiusTransition:
            return makeStyleProperty(layer.getCircleRadiusTransition());
        case CircleProperty::CircleStrokeColorTransition:
            return makeStyleProperty(layer.getCircleStrokeColorTransition());
        case CircleProperty::CircleStrokeOpacityTransition:
            return makeStyleProperty(layer.getCircleStrokeOpacityTransition());
        case CircleProperty::CircleStrokeWidthTransition:
            return makeStyleProperty(layer.getCircleStrokeWidthTransition());
        case CircleProperty::CircleTranslateTransition:
            return makeStyleProperty(layer.getCircleTranslateTransition());
        case CircleProperty::CircleTranslateAnchorTransition:
            return makeStyleProperty(layer.getCircleTranslateAnchorTransition());
        case CircleProperty::CircleSortKey:
            return makeStyleProperty(layer.getCircleSortKey());
    }
    return {};
}

StyleProperty getLayerProperty(const CircleLayer& layer, const std::string& name) {
    const auto it = circleLayerProperties.find(name.c_str());
    if (it == circleLayerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<CircleProperty>(it->second));
}

} // namespace

Value CircleLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : circleLayerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<CircleProperty>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCountCircle);
    }
    return result;
}

std::optional<Error> CircleLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = circleLayerProperties.find(name.c_str());
    if (it == circleLayerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<CircleProperty>(it->second);

    if (property == CircleProperty::CircleBlur || property == CircleProperty::CircleOpacity ||
        property == CircleProperty::CircleRadius || property == CircleProperty::CircleStrokeOpacity ||
        property == CircleProperty::CircleStrokeWidth || property == CircleProperty::CircleSortKey) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        if (property == CircleProperty::CircleBlur) {
            setCircleBlur(*typedValue);
            return std::nullopt;
        }

        if (property == CircleProperty::CircleOpacity) {
            setCircleOpacity(*typedValue);
            return std::nullopt;
        }

        if (property == CircleProperty::CircleRadius) {
            setCircleRadius(*typedValue);
            return std::nullopt;
        }

        if (property == CircleProperty::CircleStrokeOpacity) {
            setCircleStrokeOpacity(*typedValue);
            return std::nullopt;
        }

        if (property == CircleProperty::CircleStrokeWidth) {
            setCircleStrokeWidth(*typedValue);
            return std::nullopt;
        }

        if (property == CircleProperty::CircleSortKey) {
            setCircleSortKey(*typedValue);
            return std::nullopt;
        }
    }
    if (property == CircleProperty::CircleColor || property == CircleProperty::CircleStrokeColor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<Color>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        if (property == CircleProperty::CircleColor) {
            setCircleColor(*typedValue);
            return std::nullopt;
        }

        if (property == CircleProperty::CircleStrokeColor) {
            setCircleStrokeColor(*typedValue);
            return std::nullopt;
        }
    }
    if (property == CircleProperty::CirclePitchAlignment) {
        Error error;
        const auto& typedValue = convert<PropertyValue<AlignmentType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setCirclePitchAlignment(*typedValue);
        return std::nullopt;
    }
    if (property == CircleProperty::CirclePitchScale) {
        Error error;
        const auto& typedValue = convert<PropertyValue<CirclePitchScaleType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setCirclePitchScale(*typedValue);
        return std::nullopt;
    }
    if (property == CircleProperty::CircleTranslate) {
        Error error;
        const auto& typedValue = convert<PropertyValue<std::array<float, 2>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setCircleTranslate(*typedValue);
        return std::nullopt;
    }
    if (property == CircleProperty::CircleTranslateAnchor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<TranslateAnchorType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setCircleTranslateAnchor(*typedValue);
        return std::nullopt;
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == CircleProperty::CircleBlurTransition) {
        setCircleBlurTransition(*transition);
        return std::nullopt;
    }

    if (property == CircleProperty::CircleColorTransition) {
        setCircleColorTransition(*transition);
        return std::nullopt;
    }

    if (property == CircleProperty::CircleOpacityTransition) {
        setCircleOpacityTransition(*transition);
        return std::nullopt;
    }

    if (property == CircleProperty::CirclePitchAlignmentTransition) {
        setCirclePitchAlignmentTransition(*transition);
        return std::nullopt;
    }

    if (property == CircleProperty::CirclePitchScaleTransition) {
        setCirclePitchScaleTransition(*transition);
        return std::nullopt;
    }

    if (property == CircleProperty::CircleRadiusTransition) {
        setCircleRadiusTransition(*transition);
        return std::nullopt;
    }

    if (property == CircleProperty::CircleStrokeColorTransition) {
        setCircleStrokeColorTransition(*transition);
        return std::nullopt;
    }

    if (property == CircleProperty::CircleStrokeOpacityTransition) {
        setCircleStrokeOpacityTransition(*transition);
        return std::nullopt;
    }

    if (property == CircleProperty::CircleStrokeWidthTransition) {
        setCircleStrokeWidthTransition(*transition);
        return std::nullopt;
    }

    if (property == CircleProperty::CircleTranslateTransition) {
        setCircleTranslateTransition(*transition);
        return std::nullopt;
    }

    if (property == CircleProperty::CircleTranslateAnchorTransition) {
        setCircleTranslateAnchorTransition(*transition);
        return std::nullopt;
    }

    return Error{"layer doesn't support this property"};
}

StyleProperty CircleLayer::getProperty(const std::string& name) const {
    return getLayerProperty(*this, name);
}

Mutable<Layer::Impl> CircleLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl

// clang-format on
