// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/layers/background_layer_impl.hpp>
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
const LayerTypeInfo* BackgroundLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{"background",
                                        LayerTypeInfo::Source::NotRequired,
                                        LayerTypeInfo::Pass3D::NotRequired,
                                        LayerTypeInfo::Layout::NotRequired,
                                        LayerTypeInfo::FadingTiles::NotRequired,
                                        LayerTypeInfo::CrossTileIndex::NotRequired,
                                        LayerTypeInfo::TileKind::NotRequired};
    return &typeInfo;
}


BackgroundLayer::BackgroundLayer(const std::string& layerID)
    : Layer(makeMutable<Impl>(layerID, std::string())) {
}

BackgroundLayer::BackgroundLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

BackgroundLayer::~BackgroundLayer() = default;

const BackgroundLayer::Impl& BackgroundLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<BackgroundLayer::Impl> BackgroundLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> BackgroundLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    impl_->paint = BackgroundPaintProperties::Transitionable();
    return std::make_unique<BackgroundLayer>(std::move(impl_));
}

void BackgroundLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const {
}

// Layout properties


// Paint properties

PropertyValue<Color> BackgroundLayer::getDefaultBackgroundColor() {
    return {Color::black()};
}

const PropertyValue<Color>& BackgroundLayer::getBackgroundColor() const {
    return impl().paint.template get<BackgroundColor>().value;
}

void BackgroundLayer::setBackgroundColor(const PropertyValue<Color>& value) {
    if (value == getBackgroundColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<BackgroundColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void BackgroundLayer::setBackgroundColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<BackgroundColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions BackgroundLayer::getBackgroundColorTransition() const {
    return impl().paint.template get<BackgroundColor>().options;
}

PropertyValue<float> BackgroundLayer::getDefaultBackgroundOpacity() {
    return {1.f};
}

const PropertyValue<float>& BackgroundLayer::getBackgroundOpacity() const {
    return impl().paint.template get<BackgroundOpacity>().value;
}

void BackgroundLayer::setBackgroundOpacity(const PropertyValue<float>& value) {
    if (value == getBackgroundOpacity())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<BackgroundOpacity>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void BackgroundLayer::setBackgroundOpacityTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<BackgroundOpacity>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions BackgroundLayer::getBackgroundOpacityTransition() const {
    return impl().paint.template get<BackgroundOpacity>().options;
}

PropertyValue<expression::Image> BackgroundLayer::getDefaultBackgroundPattern() {
    return {{}};
}

const PropertyValue<expression::Image>& BackgroundLayer::getBackgroundPattern() const {
    return impl().paint.template get<BackgroundPattern>().value;
}

void BackgroundLayer::setBackgroundPattern(const PropertyValue<expression::Image>& value) {
    if (value == getBackgroundPattern())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<BackgroundPattern>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void BackgroundLayer::setBackgroundPatternTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<BackgroundPattern>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions BackgroundLayer::getBackgroundPatternTransition() const {
    return impl().paint.template get<BackgroundPattern>().options;
}

using namespace conversion;

namespace {


constexpr uint8_t kPaintPropertyCountBackground = 6u;


enum class BackgroundProperty : uint8_t {
    BackgroundColor,
    BackgroundOpacity,
    BackgroundPattern,
    BackgroundColorTransition,
    BackgroundOpacityTransition,
    BackgroundPatternTransition,
};


constexpr const auto backgroundLayerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"background-color", toUint8(BackgroundProperty::BackgroundColor)},
     {"background-opacity", toUint8(BackgroundProperty::BackgroundOpacity)},
     {"background-pattern", toUint8(BackgroundProperty::BackgroundPattern)},
     {"background-color-transition", toUint8(BackgroundProperty::BackgroundColorTransition)},
     {"background-opacity-transition", toUint8(BackgroundProperty::BackgroundOpacityTransition)},
     {"background-pattern-transition", toUint8(BackgroundProperty::BackgroundPatternTransition)}});

StyleProperty getLayerProperty(const BackgroundLayer& layer, BackgroundProperty property) {
    switch (property) {
        case BackgroundProperty::BackgroundColor:
            return makeStyleProperty(layer.getBackgroundColor());
        case BackgroundProperty::BackgroundOpacity:
            return makeStyleProperty(layer.getBackgroundOpacity());
        case BackgroundProperty::BackgroundPattern:
            return makeStyleProperty(layer.getBackgroundPattern());
        case BackgroundProperty::BackgroundColorTransition:
            return makeStyleProperty(layer.getBackgroundColorTransition());
        case BackgroundProperty::BackgroundOpacityTransition:
            return makeStyleProperty(layer.getBackgroundOpacityTransition());
        case BackgroundProperty::BackgroundPatternTransition:
            return makeStyleProperty(layer.getBackgroundPatternTransition());
    }
    return {};
}

StyleProperty getLayerProperty(const BackgroundLayer& layer, const std::string& name) {
    const auto it = backgroundLayerProperties.find(name.c_str());
    if (it == backgroundLayerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<BackgroundProperty>(it->second));
}

} // namespace

Value BackgroundLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : backgroundLayerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<BackgroundProperty>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCountBackground);
    }
    return result;
}

std::optional<Error> BackgroundLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = backgroundLayerProperties.find(name.c_str());
    if (it == backgroundLayerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<BackgroundProperty>(it->second);

    if (property == BackgroundProperty::BackgroundColor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<Color>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setBackgroundColor(*typedValue);
        return std::nullopt;
    }
    if (property == BackgroundProperty::BackgroundOpacity) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setBackgroundOpacity(*typedValue);
        return std::nullopt;
    }
    if (property == BackgroundProperty::BackgroundPattern) {
        Error error;
        const auto& typedValue = convert<PropertyValue<expression::Image>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setBackgroundPattern(*typedValue);
        return std::nullopt;
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == BackgroundProperty::BackgroundColorTransition) {
        setBackgroundColorTransition(*transition);
        return std::nullopt;
    }

    if (property == BackgroundProperty::BackgroundOpacityTransition) {
        setBackgroundOpacityTransition(*transition);
        return std::nullopt;
    }

    if (property == BackgroundProperty::BackgroundPatternTransition) {
        setBackgroundPatternTransition(*transition);
        return std::nullopt;
    }

    return Error{"layer doesn't support this property"};
}

StyleProperty BackgroundLayer::getProperty(const std::string& name) const {
    return getLayerProperty(*this, name);
}

Mutable<Layer::Impl> BackgroundLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl

// clang-format on
