// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/raster_layer.hpp>
#include <mbgl/style/layers/raster_layer_impl.hpp>
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
const LayerTypeInfo* RasterLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{"raster",
                                        LayerTypeInfo::Source::Required,
                                        LayerTypeInfo::Pass3D::NotRequired,
                                        LayerTypeInfo::Layout::NotRequired,
                                        LayerTypeInfo::FadingTiles::NotRequired,
                                        LayerTypeInfo::CrossTileIndex::NotRequired,
                                        LayerTypeInfo::TileKind::Raster};
    return &typeInfo;
}


RasterLayer::RasterLayer(const std::string& layerID, const std::string& sourceID)
    : Layer(makeMutable<Impl>(layerID, sourceID)) {
}

RasterLayer::RasterLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

RasterLayer::~RasterLayer() = default;

const RasterLayer::Impl& RasterLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<RasterLayer::Impl> RasterLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> RasterLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    impl_->paint = RasterPaintProperties::Transitionable();
    return std::make_unique<RasterLayer>(std::move(impl_));
}

void RasterLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const {
}

// Layout properties


// Paint properties

PropertyValue<float> RasterLayer::getDefaultRasterBrightnessMax() {
    return {1.f};
}

const PropertyValue<float>& RasterLayer::getRasterBrightnessMax() const {
    return impl().paint.template get<RasterBrightnessMax>().value;
}

void RasterLayer::setRasterBrightnessMax(const PropertyValue<float>& value) {
    if (value == getRasterBrightnessMax())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterBrightnessMax>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void RasterLayer::setRasterBrightnessMaxTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterBrightnessMax>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions RasterLayer::getRasterBrightnessMaxTransition() const {
    return impl().paint.template get<RasterBrightnessMax>().options;
}

PropertyValue<float> RasterLayer::getDefaultRasterBrightnessMin() {
    return {0.f};
}

const PropertyValue<float>& RasterLayer::getRasterBrightnessMin() const {
    return impl().paint.template get<RasterBrightnessMin>().value;
}

void RasterLayer::setRasterBrightnessMin(const PropertyValue<float>& value) {
    if (value == getRasterBrightnessMin())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterBrightnessMin>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void RasterLayer::setRasterBrightnessMinTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterBrightnessMin>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions RasterLayer::getRasterBrightnessMinTransition() const {
    return impl().paint.template get<RasterBrightnessMin>().options;
}

PropertyValue<float> RasterLayer::getDefaultRasterContrast() {
    return {0.f};
}

const PropertyValue<float>& RasterLayer::getRasterContrast() const {
    return impl().paint.template get<RasterContrast>().value;
}

void RasterLayer::setRasterContrast(const PropertyValue<float>& value) {
    if (value == getRasterContrast())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterContrast>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void RasterLayer::setRasterContrastTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterContrast>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions RasterLayer::getRasterContrastTransition() const {
    return impl().paint.template get<RasterContrast>().options;
}

PropertyValue<float> RasterLayer::getDefaultRasterFadeDuration() {
    return {300.f};
}

const PropertyValue<float>& RasterLayer::getRasterFadeDuration() const {
    return impl().paint.template get<RasterFadeDuration>().value;
}

void RasterLayer::setRasterFadeDuration(const PropertyValue<float>& value) {
    if (value == getRasterFadeDuration())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterFadeDuration>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void RasterLayer::setRasterFadeDurationTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterFadeDuration>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions RasterLayer::getRasterFadeDurationTransition() const {
    return impl().paint.template get<RasterFadeDuration>().options;
}

PropertyValue<float> RasterLayer::getDefaultRasterHueRotate() {
    return {0.f};
}

const PropertyValue<float>& RasterLayer::getRasterHueRotate() const {
    return impl().paint.template get<RasterHueRotate>().value;
}

void RasterLayer::setRasterHueRotate(const PropertyValue<float>& value) {
    if (value == getRasterHueRotate())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterHueRotate>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void RasterLayer::setRasterHueRotateTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterHueRotate>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions RasterLayer::getRasterHueRotateTransition() const {
    return impl().paint.template get<RasterHueRotate>().options;
}

PropertyValue<float> RasterLayer::getDefaultRasterOpacity() {
    return {1.f};
}

const PropertyValue<float>& RasterLayer::getRasterOpacity() const {
    return impl().paint.template get<RasterOpacity>().value;
}

void RasterLayer::setRasterOpacity(const PropertyValue<float>& value) {
    if (value == getRasterOpacity())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterOpacity>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void RasterLayer::setRasterOpacityTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterOpacity>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions RasterLayer::getRasterOpacityTransition() const {
    return impl().paint.template get<RasterOpacity>().options;
}

PropertyValue<RasterResamplingType> RasterLayer::getDefaultRasterResampling() {
    return {RasterResamplingType::Linear};
}

const PropertyValue<RasterResamplingType>& RasterLayer::getRasterResampling() const {
    return impl().paint.template get<RasterResampling>().value;
}

void RasterLayer::setRasterResampling(const PropertyValue<RasterResamplingType>& value) {
    if (value == getRasterResampling())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterResampling>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void RasterLayer::setRasterResamplingTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterResampling>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions RasterLayer::getRasterResamplingTransition() const {
    return impl().paint.template get<RasterResampling>().options;
}

PropertyValue<float> RasterLayer::getDefaultRasterSaturation() {
    return {0.f};
}

const PropertyValue<float>& RasterLayer::getRasterSaturation() const {
    return impl().paint.template get<RasterSaturation>().value;
}

void RasterLayer::setRasterSaturation(const PropertyValue<float>& value) {
    if (value == getRasterSaturation())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterSaturation>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void RasterLayer::setRasterSaturationTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<RasterSaturation>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions RasterLayer::getRasterSaturationTransition() const {
    return impl().paint.template get<RasterSaturation>().options;
}

using namespace conversion;

namespace {


constexpr uint8_t kPaintPropertyCountRaster = 16u;


enum class RasterProperty : uint8_t {
    RasterBrightnessMax,
    RasterBrightnessMin,
    RasterContrast,
    RasterFadeDuration,
    RasterHueRotate,
    RasterOpacity,
    RasterResampling,
    RasterSaturation,
    RasterBrightnessMaxTransition,
    RasterBrightnessMinTransition,
    RasterContrastTransition,
    RasterFadeDurationTransition,
    RasterHueRotateTransition,
    RasterOpacityTransition,
    RasterResamplingTransition,
    RasterSaturationTransition,
};


constexpr const auto rasterLayerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"raster-brightness-max", toUint8(RasterProperty::RasterBrightnessMax)},
     {"raster-brightness-min", toUint8(RasterProperty::RasterBrightnessMin)},
     {"raster-contrast", toUint8(RasterProperty::RasterContrast)},
     {"raster-fade-duration", toUint8(RasterProperty::RasterFadeDuration)},
     {"raster-hue-rotate", toUint8(RasterProperty::RasterHueRotate)},
     {"raster-opacity", toUint8(RasterProperty::RasterOpacity)},
     {"raster-resampling", toUint8(RasterProperty::RasterResampling)},
     {"raster-saturation", toUint8(RasterProperty::RasterSaturation)},
     {"raster-brightness-max-transition", toUint8(RasterProperty::RasterBrightnessMaxTransition)},
     {"raster-brightness-min-transition", toUint8(RasterProperty::RasterBrightnessMinTransition)},
     {"raster-contrast-transition", toUint8(RasterProperty::RasterContrastTransition)},
     {"raster-fade-duration-transition", toUint8(RasterProperty::RasterFadeDurationTransition)},
     {"raster-hue-rotate-transition", toUint8(RasterProperty::RasterHueRotateTransition)},
     {"raster-opacity-transition", toUint8(RasterProperty::RasterOpacityTransition)},
     {"raster-resampling-transition", toUint8(RasterProperty::RasterResamplingTransition)},
     {"raster-saturation-transition", toUint8(RasterProperty::RasterSaturationTransition)}});

StyleProperty getLayerProperty(const RasterLayer& layer, RasterProperty property) {
    switch (property) {
        case RasterProperty::RasterBrightnessMax:
            return makeStyleProperty(layer.getRasterBrightnessMax());
        case RasterProperty::RasterBrightnessMin:
            return makeStyleProperty(layer.getRasterBrightnessMin());
        case RasterProperty::RasterContrast:
            return makeStyleProperty(layer.getRasterContrast());
        case RasterProperty::RasterFadeDuration:
            return makeStyleProperty(layer.getRasterFadeDuration());
        case RasterProperty::RasterHueRotate:
            return makeStyleProperty(layer.getRasterHueRotate());
        case RasterProperty::RasterOpacity:
            return makeStyleProperty(layer.getRasterOpacity());
        case RasterProperty::RasterResampling:
            return makeStyleProperty(layer.getRasterResampling());
        case RasterProperty::RasterSaturation:
            return makeStyleProperty(layer.getRasterSaturation());
        case RasterProperty::RasterBrightnessMaxTransition:
            return makeStyleProperty(layer.getRasterBrightnessMaxTransition());
        case RasterProperty::RasterBrightnessMinTransition:
            return makeStyleProperty(layer.getRasterBrightnessMinTransition());
        case RasterProperty::RasterContrastTransition:
            return makeStyleProperty(layer.getRasterContrastTransition());
        case RasterProperty::RasterFadeDurationTransition:
            return makeStyleProperty(layer.getRasterFadeDurationTransition());
        case RasterProperty::RasterHueRotateTransition:
            return makeStyleProperty(layer.getRasterHueRotateTransition());
        case RasterProperty::RasterOpacityTransition:
            return makeStyleProperty(layer.getRasterOpacityTransition());
        case RasterProperty::RasterResamplingTransition:
            return makeStyleProperty(layer.getRasterResamplingTransition());
        case RasterProperty::RasterSaturationTransition:
            return makeStyleProperty(layer.getRasterSaturationTransition());
    }
    return {};
}

StyleProperty getLayerProperty(const RasterLayer& layer, const std::string& name) {
    const auto it = rasterLayerProperties.find(name.c_str());
    if (it == rasterLayerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<RasterProperty>(it->second));
}

} // namespace

Value RasterLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : rasterLayerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<RasterProperty>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCountRaster);
    }
    return result;
}

std::optional<Error> RasterLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = rasterLayerProperties.find(name.c_str());
    if (it == rasterLayerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<RasterProperty>(it->second);

    if (property == RasterProperty::RasterBrightnessMax || property == RasterProperty::RasterBrightnessMin ||
        property == RasterProperty::RasterContrast || property == RasterProperty::RasterFadeDuration ||
        property == RasterProperty::RasterHueRotate || property == RasterProperty::RasterOpacity ||
        property == RasterProperty::RasterSaturation) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == RasterProperty::RasterBrightnessMax) {
            setRasterBrightnessMax(*typedValue);
            return std::nullopt;
        }

        if (property == RasterProperty::RasterBrightnessMin) {
            setRasterBrightnessMin(*typedValue);
            return std::nullopt;
        }

        if (property == RasterProperty::RasterContrast) {
            setRasterContrast(*typedValue);
            return std::nullopt;
        }

        if (property == RasterProperty::RasterFadeDuration) {
            setRasterFadeDuration(*typedValue);
            return std::nullopt;
        }

        if (property == RasterProperty::RasterHueRotate) {
            setRasterHueRotate(*typedValue);
            return std::nullopt;
        }

        if (property == RasterProperty::RasterOpacity) {
            setRasterOpacity(*typedValue);
            return std::nullopt;
        }

        if (property == RasterProperty::RasterSaturation) {
            setRasterSaturation(*typedValue);
            return std::nullopt;
        }
    }
    if (property == RasterProperty::RasterResampling) {
        Error error;
        const auto& typedValue = convert<PropertyValue<RasterResamplingType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setRasterResampling(*typedValue);
        return std::nullopt;
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == RasterProperty::RasterBrightnessMaxTransition) {
        setRasterBrightnessMaxTransition(*transition);
        return std::nullopt;
    }

    if (property == RasterProperty::RasterBrightnessMinTransition) {
        setRasterBrightnessMinTransition(*transition);
        return std::nullopt;
    }

    if (property == RasterProperty::RasterContrastTransition) {
        setRasterContrastTransition(*transition);
        return std::nullopt;
    }

    if (property == RasterProperty::RasterFadeDurationTransition) {
        setRasterFadeDurationTransition(*transition);
        return std::nullopt;
    }

    if (property == RasterProperty::RasterHueRotateTransition) {
        setRasterHueRotateTransition(*transition);
        return std::nullopt;
    }

    if (property == RasterProperty::RasterOpacityTransition) {
        setRasterOpacityTransition(*transition);
        return std::nullopt;
    }

    if (property == RasterProperty::RasterResamplingTransition) {
        setRasterResamplingTransition(*transition);
        return std::nullopt;
    }

    if (property == RasterProperty::RasterSaturationTransition) {
        setRasterSaturationTransition(*transition);
        return std::nullopt;
    }

    return Error{"layer doesn't support this property"};
}

StyleProperty RasterLayer::getProperty(const std::string& name) const {
    return getLayerProperty(*this, name);
}

Mutable<Layer::Impl> RasterLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl

// clang-format on
