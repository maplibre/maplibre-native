// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/fill_extrusion_layer.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_impl.hpp>
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
const LayerTypeInfo* FillExtrusionLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{"fill-extrusion",
                                        LayerTypeInfo::Source::Required,
                                        LayerTypeInfo::Pass3D::Required,
                                        LayerTypeInfo::Layout::Required,
                                        LayerTypeInfo::FadingTiles::NotRequired,
                                        LayerTypeInfo::CrossTileIndex::NotRequired,
                                        LayerTypeInfo::TileKind::Geometry};
    return &typeInfo;
}


FillExtrusionLayer::FillExtrusionLayer(const std::string& layerID, const std::string& sourceID)
    : Layer(makeMutable<Impl>(layerID, sourceID)) {
}

FillExtrusionLayer::FillExtrusionLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

FillExtrusionLayer::~FillExtrusionLayer() = default;

const FillExtrusionLayer::Impl& FillExtrusionLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<FillExtrusionLayer::Impl> FillExtrusionLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> FillExtrusionLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    impl_->paint = FillExtrusionPaintProperties::Transitionable();
    return std::make_unique<FillExtrusionLayer>(std::move(impl_));
}

void FillExtrusionLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const {
}

// Layout properties


// Paint properties

PropertyValue<float> FillExtrusionLayer::getDefaultFillExtrusionBase() {
    return {0.f};
}

const PropertyValue<float>& FillExtrusionLayer::getFillExtrusionBase() const {
    return impl().paint.template get<FillExtrusionBase>().value;
}

void FillExtrusionLayer::setFillExtrusionBase(const PropertyValue<float>& value) {
    if (value == getFillExtrusionBase())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionBase>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillExtrusionLayer::setFillExtrusionBaseTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionBase>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillExtrusionLayer::getFillExtrusionBaseTransition() const {
    return impl().paint.template get<FillExtrusionBase>().options;
}

PropertyValue<Color> FillExtrusionLayer::getDefaultFillExtrusionColor() {
    return {Color::black()};
}

const PropertyValue<Color>& FillExtrusionLayer::getFillExtrusionColor() const {
    return impl().paint.template get<FillExtrusionColor>().value;
}

void FillExtrusionLayer::setFillExtrusionColor(const PropertyValue<Color>& value) {
    if (value == getFillExtrusionColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillExtrusionLayer::setFillExtrusionColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillExtrusionLayer::getFillExtrusionColorTransition() const {
    return impl().paint.template get<FillExtrusionColor>().options;
}

PropertyValue<float> FillExtrusionLayer::getDefaultFillExtrusionHeight() {
    return {0.f};
}

const PropertyValue<float>& FillExtrusionLayer::getFillExtrusionHeight() const {
    return impl().paint.template get<FillExtrusionHeight>().value;
}

void FillExtrusionLayer::setFillExtrusionHeight(const PropertyValue<float>& value) {
    if (value == getFillExtrusionHeight())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionHeight>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillExtrusionLayer::setFillExtrusionHeightTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionHeight>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillExtrusionLayer::getFillExtrusionHeightTransition() const {
    return impl().paint.template get<FillExtrusionHeight>().options;
}

PropertyValue<float> FillExtrusionLayer::getDefaultFillExtrusionOpacity() {
    return {1.f};
}

const PropertyValue<float>& FillExtrusionLayer::getFillExtrusionOpacity() const {
    return impl().paint.template get<FillExtrusionOpacity>().value;
}

void FillExtrusionLayer::setFillExtrusionOpacity(const PropertyValue<float>& value) {
    if (value == getFillExtrusionOpacity())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionOpacity>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillExtrusionLayer::setFillExtrusionOpacityTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionOpacity>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillExtrusionLayer::getFillExtrusionOpacityTransition() const {
    return impl().paint.template get<FillExtrusionOpacity>().options;
}

PropertyValue<expression::Image> FillExtrusionLayer::getDefaultFillExtrusionPattern() {
    return {{}};
}

const PropertyValue<expression::Image>& FillExtrusionLayer::getFillExtrusionPattern() const {
    return impl().paint.template get<FillExtrusionPattern>().value;
}

void FillExtrusionLayer::setFillExtrusionPattern(const PropertyValue<expression::Image>& value) {
    if (value == getFillExtrusionPattern())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionPattern>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillExtrusionLayer::setFillExtrusionPatternTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionPattern>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillExtrusionLayer::getFillExtrusionPatternTransition() const {
    return impl().paint.template get<FillExtrusionPattern>().options;
}

PropertyValue<std::array<float, 2>> FillExtrusionLayer::getDefaultFillExtrusionTranslate() {
    return {{{0.f, 0.f}}};
}

const PropertyValue<std::array<float, 2>>& FillExtrusionLayer::getFillExtrusionTranslate() const {
    return impl().paint.template get<FillExtrusionTranslate>().value;
}

void FillExtrusionLayer::setFillExtrusionTranslate(const PropertyValue<std::array<float, 2>>& value) {
    if (value == getFillExtrusionTranslate())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionTranslate>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillExtrusionLayer::setFillExtrusionTranslateTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionTranslate>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillExtrusionLayer::getFillExtrusionTranslateTransition() const {
    return impl().paint.template get<FillExtrusionTranslate>().options;
}

PropertyValue<TranslateAnchorType> FillExtrusionLayer::getDefaultFillExtrusionTranslateAnchor() {
    return {TranslateAnchorType::Map};
}

const PropertyValue<TranslateAnchorType>& FillExtrusionLayer::getFillExtrusionTranslateAnchor() const {
    return impl().paint.template get<FillExtrusionTranslateAnchor>().value;
}

void FillExtrusionLayer::setFillExtrusionTranslateAnchor(const PropertyValue<TranslateAnchorType>& value) {
    if (value == getFillExtrusionTranslateAnchor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionTranslateAnchor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillExtrusionLayer::setFillExtrusionTranslateAnchorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionTranslateAnchor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillExtrusionLayer::getFillExtrusionTranslateAnchorTransition() const {
    return impl().paint.template get<FillExtrusionTranslateAnchor>().options;
}

PropertyValue<bool> FillExtrusionLayer::getDefaultFillExtrusionVerticalGradient() {
    return {true};
}

const PropertyValue<bool>& FillExtrusionLayer::getFillExtrusionVerticalGradient() const {
    return impl().paint.template get<FillExtrusionVerticalGradient>().value;
}

void FillExtrusionLayer::setFillExtrusionVerticalGradient(const PropertyValue<bool>& value) {
    if (value == getFillExtrusionVerticalGradient())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionVerticalGradient>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillExtrusionLayer::setFillExtrusionVerticalGradientTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillExtrusionVerticalGradient>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillExtrusionLayer::getFillExtrusionVerticalGradientTransition() const {
    return impl().paint.template get<FillExtrusionVerticalGradient>().options;
}

using namespace conversion;

namespace {


constexpr uint8_t kPaintPropertyCountFillExtrusion = 16u;


enum class FillExtrusionProperty : uint8_t {
    FillExtrusionBase,
    FillExtrusionColor,
    FillExtrusionHeight,
    FillExtrusionOpacity,
    FillExtrusionPattern,
    FillExtrusionTranslate,
    FillExtrusionTranslateAnchor,
    FillExtrusionVerticalGradient,
    FillExtrusionBaseTransition,
    FillExtrusionColorTransition,
    FillExtrusionHeightTransition,
    FillExtrusionOpacityTransition,
    FillExtrusionPatternTransition,
    FillExtrusionTranslateTransition,
    FillExtrusionTranslateAnchorTransition,
    FillExtrusionVerticalGradientTransition,
};


constexpr const auto fillExtrusionLayerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"fill-extrusion-base", toUint8(FillExtrusionProperty::FillExtrusionBase)},
     {"fill-extrusion-color", toUint8(FillExtrusionProperty::FillExtrusionColor)},
     {"fill-extrusion-height", toUint8(FillExtrusionProperty::FillExtrusionHeight)},
     {"fill-extrusion-opacity", toUint8(FillExtrusionProperty::FillExtrusionOpacity)},
     {"fill-extrusion-pattern", toUint8(FillExtrusionProperty::FillExtrusionPattern)},
     {"fill-extrusion-translate", toUint8(FillExtrusionProperty::FillExtrusionTranslate)},
     {"fill-extrusion-translate-anchor", toUint8(FillExtrusionProperty::FillExtrusionTranslateAnchor)},
     {"fill-extrusion-vertical-gradient", toUint8(FillExtrusionProperty::FillExtrusionVerticalGradient)},
     {"fill-extrusion-base-transition", toUint8(FillExtrusionProperty::FillExtrusionBaseTransition)},
     {"fill-extrusion-color-transition", toUint8(FillExtrusionProperty::FillExtrusionColorTransition)},
     {"fill-extrusion-height-transition", toUint8(FillExtrusionProperty::FillExtrusionHeightTransition)},
     {"fill-extrusion-opacity-transition", toUint8(FillExtrusionProperty::FillExtrusionOpacityTransition)},
     {"fill-extrusion-pattern-transition", toUint8(FillExtrusionProperty::FillExtrusionPatternTransition)},
     {"fill-extrusion-translate-transition", toUint8(FillExtrusionProperty::FillExtrusionTranslateTransition)},
     {"fill-extrusion-translate-anchor-transition", toUint8(FillExtrusionProperty::FillExtrusionTranslateAnchorTransition)},
     {"fill-extrusion-vertical-gradient-transition", toUint8(FillExtrusionProperty::FillExtrusionVerticalGradientTransition)}});

StyleProperty getLayerProperty(const FillExtrusionLayer& layer, FillExtrusionProperty property) {
    switch (property) {
        case FillExtrusionProperty::FillExtrusionBase:
            return makeStyleProperty(layer.getFillExtrusionBase());
        case FillExtrusionProperty::FillExtrusionColor:
            return makeStyleProperty(layer.getFillExtrusionColor());
        case FillExtrusionProperty::FillExtrusionHeight:
            return makeStyleProperty(layer.getFillExtrusionHeight());
        case FillExtrusionProperty::FillExtrusionOpacity:
            return makeStyleProperty(layer.getFillExtrusionOpacity());
        case FillExtrusionProperty::FillExtrusionPattern:
            return makeStyleProperty(layer.getFillExtrusionPattern());
        case FillExtrusionProperty::FillExtrusionTranslate:
            return makeStyleProperty(layer.getFillExtrusionTranslate());
        case FillExtrusionProperty::FillExtrusionTranslateAnchor:
            return makeStyleProperty(layer.getFillExtrusionTranslateAnchor());
        case FillExtrusionProperty::FillExtrusionVerticalGradient:
            return makeStyleProperty(layer.getFillExtrusionVerticalGradient());
        case FillExtrusionProperty::FillExtrusionBaseTransition:
            return makeStyleProperty(layer.getFillExtrusionBaseTransition());
        case FillExtrusionProperty::FillExtrusionColorTransition:
            return makeStyleProperty(layer.getFillExtrusionColorTransition());
        case FillExtrusionProperty::FillExtrusionHeightTransition:
            return makeStyleProperty(layer.getFillExtrusionHeightTransition());
        case FillExtrusionProperty::FillExtrusionOpacityTransition:
            return makeStyleProperty(layer.getFillExtrusionOpacityTransition());
        case FillExtrusionProperty::FillExtrusionPatternTransition:
            return makeStyleProperty(layer.getFillExtrusionPatternTransition());
        case FillExtrusionProperty::FillExtrusionTranslateTransition:
            return makeStyleProperty(layer.getFillExtrusionTranslateTransition());
        case FillExtrusionProperty::FillExtrusionTranslateAnchorTransition:
            return makeStyleProperty(layer.getFillExtrusionTranslateAnchorTransition());
        case FillExtrusionProperty::FillExtrusionVerticalGradientTransition:
            return makeStyleProperty(layer.getFillExtrusionVerticalGradientTransition());
    }
    return {};
}

StyleProperty getLayerProperty(const FillExtrusionLayer& layer, const std::string& name) {
    const auto it = fillExtrusionLayerProperties.find(name.c_str());
    if (it == fillExtrusionLayerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<FillExtrusionProperty>(it->second));
}

} // namespace

Value FillExtrusionLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : fillExtrusionLayerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<FillExtrusionProperty>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCountFillExtrusion);
    }
    return result;
}

std::optional<Error> FillExtrusionLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = fillExtrusionLayerProperties.find(name.c_str());
    if (it == fillExtrusionLayerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<FillExtrusionProperty>(it->second);

    if (property == FillExtrusionProperty::FillExtrusionBase ||
        property == FillExtrusionProperty::FillExtrusionHeight) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        if (property == FillExtrusionProperty::FillExtrusionBase) {
            setFillExtrusionBase(*typedValue);
            return std::nullopt;
        }

        if (property == FillExtrusionProperty::FillExtrusionHeight) {
            setFillExtrusionHeight(*typedValue);
            return std::nullopt;
        }
    }
    if (property == FillExtrusionProperty::FillExtrusionColor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<Color>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setFillExtrusionColor(*typedValue);
        return std::nullopt;
    }
    if (property == FillExtrusionProperty::FillExtrusionOpacity) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setFillExtrusionOpacity(*typedValue);
        return std::nullopt;
    }
    if (property == FillExtrusionProperty::FillExtrusionPattern) {
        Error error;
        const auto& typedValue = convert<PropertyValue<expression::Image>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setFillExtrusionPattern(*typedValue);
        return std::nullopt;
    }
    if (property == FillExtrusionProperty::FillExtrusionTranslate) {
        Error error;
        const auto& typedValue = convert<PropertyValue<std::array<float, 2>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setFillExtrusionTranslate(*typedValue);
        return std::nullopt;
    }
    if (property == FillExtrusionProperty::FillExtrusionTranslateAnchor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<TranslateAnchorType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setFillExtrusionTranslateAnchor(*typedValue);
        return std::nullopt;
    }
    if (property == FillExtrusionProperty::FillExtrusionVerticalGradient) {
        Error error;
        const auto& typedValue = convert<PropertyValue<bool>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setFillExtrusionVerticalGradient(*typedValue);
        return std::nullopt;
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == FillExtrusionProperty::FillExtrusionBaseTransition) {
        setFillExtrusionBaseTransition(*transition);
        return std::nullopt;
    }

    if (property == FillExtrusionProperty::FillExtrusionColorTransition) {
        setFillExtrusionColorTransition(*transition);
        return std::nullopt;
    }

    if (property == FillExtrusionProperty::FillExtrusionHeightTransition) {
        setFillExtrusionHeightTransition(*transition);
        return std::nullopt;
    }

    if (property == FillExtrusionProperty::FillExtrusionOpacityTransition) {
        setFillExtrusionOpacityTransition(*transition);
        return std::nullopt;
    }

    if (property == FillExtrusionProperty::FillExtrusionPatternTransition) {
        setFillExtrusionPatternTransition(*transition);
        return std::nullopt;
    }

    if (property == FillExtrusionProperty::FillExtrusionTranslateTransition) {
        setFillExtrusionTranslateTransition(*transition);
        return std::nullopt;
    }

    if (property == FillExtrusionProperty::FillExtrusionTranslateAnchorTransition) {
        setFillExtrusionTranslateAnchorTransition(*transition);
        return std::nullopt;
    }

    if (property == FillExtrusionProperty::FillExtrusionVerticalGradientTransition) {
        setFillExtrusionVerticalGradientTransition(*transition);
        return std::nullopt;
    }

    return Error{"layer doesn't support this property"};
}

StyleProperty FillExtrusionLayer::getProperty(const std::string& name) const {
    return getLayerProperty(*this, name);
}

Mutable<Layer::Impl> FillExtrusionLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl

// clang-format on
