// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/debug_layer.hpp>
#include <mbgl/style/layers/debug_layer_impl.hpp>
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
const LayerTypeInfo* DebugLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{"debug",
                                        LayerTypeInfo::Source::NotRequired,
                                        LayerTypeInfo::Pass3D::NotRequired,
                                        LayerTypeInfo::Layout::NotRequired,
                                        LayerTypeInfo::FadingTiles::NotRequired,
                                        LayerTypeInfo::CrossTileIndex::NotRequired,
                                        LayerTypeInfo::TileKind::NotRequired};
    return &typeInfo;
}


DebugLayer::DebugLayer(const std::string& layerID)
    : Layer(makeMutable<Impl>(layerID, std::string())) {
}

DebugLayer::DebugLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

DebugLayer::~DebugLayer() = default;

const DebugLayer::Impl& DebugLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<DebugLayer::Impl> DebugLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> DebugLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    impl_->paint = DebugPaintProperties::Transitionable();
    return std::make_unique<DebugLayer>(std::move(impl_));
}

void DebugLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const {
}

// Layout properties


// Paint properties

PropertyValue<Color> DebugLayer::getDefaultBorderColor() {
    return {{ 1, 0, 0, 1 }};
}

const PropertyValue<Color>& DebugLayer::getBorderColor() const {
    return impl().paint.template get<BorderColor>().value;
}

void DebugLayer::setBorderColor(const PropertyValue<Color>& value) {
    if (value == getBorderColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<BorderColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void DebugLayer::setBorderColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<BorderColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions DebugLayer::getBorderColorTransition() const {
    return impl().paint.template get<BorderColor>().options;
}

using namespace conversion;

namespace {

constexpr uint8_t kPaintPropertyCount = 2u;

enum class Property : uint8_t {
    BorderColor,
    BorderColorTransition,
};

template <typename T>
constexpr uint8_t toUint8(T t) noexcept {
    return uint8_t(mbgl::underlying_type(t));
}

MAPBOX_ETERNAL_CONSTEXPR const auto layerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"border-color", toUint8(Property::BorderColor)},
     {"border-color-transition", toUint8(Property::BorderColorTransition)}});

StyleProperty getLayerProperty(const DebugLayer& layer, Property property) {
    switch (property) {
        case Property::BorderColor:
            return makeStyleProperty(layer.getBorderColor());
        case Property::BorderColorTransition:
            return makeStyleProperty(layer.getBorderColorTransition());
    }
    return {};
}

StyleProperty getLayerProperty(const DebugLayer& layer, const std::string& name) {
    const auto it = layerProperties.find(name.c_str());
    if (it == layerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<Property>(it->second));
}

} // namespace

Value DebugLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : layerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<Property>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCount);
    }
    return result;
}

std::optional<Error> DebugLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = layerProperties.find(name.c_str());
    if (it == layerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<Property>(it->second);

    if (property == Property::BorderColor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<Color>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setBorderColor(*typedValue);
        return std::nullopt;
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == Property::BorderColorTransition) {
        setBorderColorTransition(*transition);
        return std::nullopt;
    }

    return Error{"layer doesn't support this property"};
}

StyleProperty DebugLayer::getProperty(const std::string& name) const {
    return getLayerProperty(*this, name);
}

Mutable<Layer::Impl> DebugLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl

// clang-format on
