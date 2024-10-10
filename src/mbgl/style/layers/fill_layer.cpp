// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
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
const LayerTypeInfo* FillLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{"fill",
                                        LayerTypeInfo::Source::Required,
                                        LayerTypeInfo::Pass3D::NotRequired,
                                        LayerTypeInfo::Layout::Required,
                                        LayerTypeInfo::FadingTiles::NotRequired,
                                        LayerTypeInfo::CrossTileIndex::NotRequired,
                                        LayerTypeInfo::TileKind::Geometry};
    return &typeInfo;
}


FillLayer::FillLayer(const std::string& layerID, const std::string& sourceID)
    : Layer(makeMutable<Impl>(layerID, sourceID)) {
}

FillLayer::FillLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

FillLayer::~FillLayer() = default;

const FillLayer::Impl& FillLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<FillLayer::Impl> FillLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> FillLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    impl_->paint = FillPaintProperties::Transitionable();
    return std::make_unique<FillLayer>(std::move(impl_));
}

void FillLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>& writer) const {
    layout.stringify(writer);
}

// Layout properties

PropertyValue<float> FillLayer::getDefaultFillSortKey() {
    return FillSortKey::defaultValue();
}

const PropertyValue<float>& FillLayer::getFillSortKey() const {
    return impl().layout.get<FillSortKey>();
}

void FillLayer::setFillSortKey(const PropertyValue<float>& value) {
    if (value == getFillSortKey()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<FillSortKey>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

// Paint properties

PropertyValue<bool> FillLayer::getDefaultFillAntialias() {
    return {true};
}

const PropertyValue<bool>& FillLayer::getFillAntialias() const {
    return impl().paint.template get<FillAntialias>().value;
}

void FillLayer::setFillAntialias(const PropertyValue<bool>& value) {
    if (value == getFillAntialias())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillAntialias>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillLayer::setFillAntialiasTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillAntialias>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillLayer::getFillAntialiasTransition() const {
    return impl().paint.template get<FillAntialias>().options;
}

PropertyValue<Color> FillLayer::getDefaultFillColor() {
    return {Color::black()};
}

const PropertyValue<Color>& FillLayer::getFillColor() const {
    return impl().paint.template get<FillColor>().value;
}

void FillLayer::setFillColor(const PropertyValue<Color>& value) {
    if (value == getFillColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillLayer::setFillColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillLayer::getFillColorTransition() const {
    return impl().paint.template get<FillColor>().options;
}

PropertyValue<float> FillLayer::getDefaultFillOpacity() {
    return {1.f};
}

const PropertyValue<float>& FillLayer::getFillOpacity() const {
    return impl().paint.template get<FillOpacity>().value;
}

void FillLayer::setFillOpacity(const PropertyValue<float>& value) {
    if (value == getFillOpacity())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillOpacity>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillLayer::setFillOpacityTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillOpacity>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillLayer::getFillOpacityTransition() const {
    return impl().paint.template get<FillOpacity>().options;
}

PropertyValue<Color> FillLayer::getDefaultFillOutlineColor() {
    return {{}};
}

const PropertyValue<Color>& FillLayer::getFillOutlineColor() const {
    return impl().paint.template get<FillOutlineColor>().value;
}

void FillLayer::setFillOutlineColor(const PropertyValue<Color>& value) {
    if (value == getFillOutlineColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillOutlineColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillLayer::setFillOutlineColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillOutlineColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillLayer::getFillOutlineColorTransition() const {
    return impl().paint.template get<FillOutlineColor>().options;
}

PropertyValue<expression::Image> FillLayer::getDefaultFillPattern() {
    return {{}};
}

const PropertyValue<expression::Image>& FillLayer::getFillPattern() const {
    return impl().paint.template get<FillPattern>().value;
}

void FillLayer::setFillPattern(const PropertyValue<expression::Image>& value) {
    if (value == getFillPattern())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillPattern>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillLayer::setFillPatternTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillPattern>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillLayer::getFillPatternTransition() const {
    return impl().paint.template get<FillPattern>().options;
}

PropertyValue<std::array<float, 2>> FillLayer::getDefaultFillTranslate() {
    return {{{0.f, 0.f}}};
}

const PropertyValue<std::array<float, 2>>& FillLayer::getFillTranslate() const {
    return impl().paint.template get<FillTranslate>().value;
}

void FillLayer::setFillTranslate(const PropertyValue<std::array<float, 2>>& value) {
    if (value == getFillTranslate())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillTranslate>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillLayer::setFillTranslateTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillTranslate>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillLayer::getFillTranslateTransition() const {
    return impl().paint.template get<FillTranslate>().options;
}

PropertyValue<TranslateAnchorType> FillLayer::getDefaultFillTranslateAnchor() {
    return {TranslateAnchorType::Map};
}

const PropertyValue<TranslateAnchorType>& FillLayer::getFillTranslateAnchor() const {
    return impl().paint.template get<FillTranslateAnchor>().value;
}

void FillLayer::setFillTranslateAnchor(const PropertyValue<TranslateAnchorType>& value) {
    if (value == getFillTranslateAnchor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillTranslateAnchor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void FillLayer::setFillTranslateAnchorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<FillTranslateAnchor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions FillLayer::getFillTranslateAnchorTransition() const {
    return impl().paint.template get<FillTranslateAnchor>().options;
}

using namespace conversion;

namespace {


constexpr uint8_t kPaintPropertyCountFill = 14u;


enum class FillProperty : uint8_t {
    FillAntialias,
    FillColor,
    FillOpacity,
    FillOutlineColor,
    FillPattern,
    FillTranslate,
    FillTranslateAnchor,
    FillAntialiasTransition,
    FillColorTransition,
    FillOpacityTransition,
    FillOutlineColorTransition,
    FillPatternTransition,
    FillTranslateTransition,
    FillTranslateAnchorTransition,
    FillSortKey = kPaintPropertyCountFill,
};


constexpr const auto fillLayerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"fill-antialias", toUint8(FillProperty::FillAntialias)},
     {"fill-color", toUint8(FillProperty::FillColor)},
     {"fill-opacity", toUint8(FillProperty::FillOpacity)},
     {"fill-outline-color", toUint8(FillProperty::FillOutlineColor)},
     {"fill-pattern", toUint8(FillProperty::FillPattern)},
     {"fill-translate", toUint8(FillProperty::FillTranslate)},
     {"fill-translate-anchor", toUint8(FillProperty::FillTranslateAnchor)},
     {"fill-antialias-transition", toUint8(FillProperty::FillAntialiasTransition)},
     {"fill-color-transition", toUint8(FillProperty::FillColorTransition)},
     {"fill-opacity-transition", toUint8(FillProperty::FillOpacityTransition)},
     {"fill-outline-color-transition", toUint8(FillProperty::FillOutlineColorTransition)},
     {"fill-pattern-transition", toUint8(FillProperty::FillPatternTransition)},
     {"fill-translate-transition", toUint8(FillProperty::FillTranslateTransition)},
     {"fill-translate-anchor-transition", toUint8(FillProperty::FillTranslateAnchorTransition)},
     {"fill-sort-key", toUint8(FillProperty::FillSortKey)}});

StyleProperty getLayerProperty(const FillLayer& layer, FillProperty property) {
    switch (property) {
        case FillProperty::FillAntialias:
            return makeStyleProperty(layer.getFillAntialias());
        case FillProperty::FillColor:
            return makeStyleProperty(layer.getFillColor());
        case FillProperty::FillOpacity:
            return makeStyleProperty(layer.getFillOpacity());
        case FillProperty::FillOutlineColor:
            return makeStyleProperty(layer.getFillOutlineColor());
        case FillProperty::FillPattern:
            return makeStyleProperty(layer.getFillPattern());
        case FillProperty::FillTranslate:
            return makeStyleProperty(layer.getFillTranslate());
        case FillProperty::FillTranslateAnchor:
            return makeStyleProperty(layer.getFillTranslateAnchor());
        case FillProperty::FillAntialiasTransition:
            return makeStyleProperty(layer.getFillAntialiasTransition());
        case FillProperty::FillColorTransition:
            return makeStyleProperty(layer.getFillColorTransition());
        case FillProperty::FillOpacityTransition:
            return makeStyleProperty(layer.getFillOpacityTransition());
        case FillProperty::FillOutlineColorTransition:
            return makeStyleProperty(layer.getFillOutlineColorTransition());
        case FillProperty::FillPatternTransition:
            return makeStyleProperty(layer.getFillPatternTransition());
        case FillProperty::FillTranslateTransition:
            return makeStyleProperty(layer.getFillTranslateTransition());
        case FillProperty::FillTranslateAnchorTransition:
            return makeStyleProperty(layer.getFillTranslateAnchorTransition());
        case FillProperty::FillSortKey:
            return makeStyleProperty(layer.getFillSortKey());
    }
    return {};
}

StyleProperty getLayerProperty(const FillLayer& layer, const std::string& name) {
    const auto it = fillLayerProperties.find(name.c_str());
    if (it == fillLayerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<FillProperty>(it->second));
}

} // namespace

Value FillLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : fillLayerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<FillProperty>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCountFill);
    }
    return result;
}

std::optional<Error> FillLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = fillLayerProperties.find(name.c_str());
    if (it == fillLayerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<FillProperty>(it->second);

    if (property == FillProperty::FillAntialias) {
        Error error;
        const auto& typedValue = convert<PropertyValue<bool>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setFillAntialias(*typedValue);
        return std::nullopt;
    }
    if (property == FillProperty::FillColor || property == FillProperty::FillOutlineColor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<Color>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        if (property == FillProperty::FillColor) {
            setFillColor(*typedValue);
            return std::nullopt;
        }

        if (property == FillProperty::FillOutlineColor) {
            setFillOutlineColor(*typedValue);
            return std::nullopt;
        }
    }
    if (property == FillProperty::FillOpacity || property == FillProperty::FillSortKey) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        if (property == FillProperty::FillOpacity) {
            setFillOpacity(*typedValue);
            return std::nullopt;
        }

        if (property == FillProperty::FillSortKey) {
            setFillSortKey(*typedValue);
            return std::nullopt;
        }
    }
    if (property == FillProperty::FillPattern) {
        Error error;
        const auto& typedValue = convert<PropertyValue<expression::Image>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setFillPattern(*typedValue);
        return std::nullopt;
    }
    if (property == FillProperty::FillTranslate) {
        Error error;
        const auto& typedValue = convert<PropertyValue<std::array<float, 2>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setFillTranslate(*typedValue);
        return std::nullopt;
    }
    if (property == FillProperty::FillTranslateAnchor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<TranslateAnchorType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setFillTranslateAnchor(*typedValue);
        return std::nullopt;
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == FillProperty::FillAntialiasTransition) {
        setFillAntialiasTransition(*transition);
        return std::nullopt;
    }

    if (property == FillProperty::FillColorTransition) {
        setFillColorTransition(*transition);
        return std::nullopt;
    }

    if (property == FillProperty::FillOpacityTransition) {
        setFillOpacityTransition(*transition);
        return std::nullopt;
    }

    if (property == FillProperty::FillOutlineColorTransition) {
        setFillOutlineColorTransition(*transition);
        return std::nullopt;
    }

    if (property == FillProperty::FillPatternTransition) {
        setFillPatternTransition(*transition);
        return std::nullopt;
    }

    if (property == FillProperty::FillTranslateTransition) {
        setFillTranslateTransition(*transition);
        return std::nullopt;
    }

    if (property == FillProperty::FillTranslateAnchorTransition) {
        setFillTranslateAnchorTransition(*transition);
        return std::nullopt;
    }

    return Error{"layer doesn't support this property"};
}

StyleProperty FillLayer::getProperty(const std::string& name) const {
    return getLayerProperty(*this, name);
}

Mutable<Layer::Impl> FillLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl

// clang-format on
