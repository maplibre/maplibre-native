// This file is generated. Do not edit.

#include <mln/style/sky.hpp>
#include <mln/style/sky_impl.hpp>
#include <mln/style/sky_observer.hpp>
#include <mln/style/conversion/sky.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/style/conversion/transition_options.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/util/traits.hpp>

#include <mapbox/eternal.hpp>

#include <utility>

namespace mln {
namespace style {

namespace {
SkyObserver nullObserver;
} // namespace

Sky::Sky(Immutable<Sky::Impl> impl_)
    : impl(std::move(impl_)),
      observer(&nullObserver) {}

Sky::Sky()
    : Sky(makeMutable<Impl>()) {}

Sky::~Sky() = default;

void Sky::setObserver(SkyObserver* observer_) {
    observer = observer_ ? observer_ : &nullObserver;
}

Mutable<Sky::Impl> Sky::mutableImpl() const {
    return makeMutable<Impl>(*impl);
}

using namespace conversion;

namespace {

enum class Property : uint8_t {
    AtmosphereBlend,
    FogColor,
    FogGroundBlend,
    HorizonColor,
    HorizonFogBlend,
    SkyColor,
    SkyHorizonBlend,
    AtmosphereBlendTransition,
    FogColorTransition,
    FogGroundBlendTransition,
    HorizonColorTransition,
    HorizonFogBlendTransition,
    SkyColorTransition,
    SkyHorizonBlendTransition,
};

template <typename T>
constexpr uint8_t toUint8(T t) noexcept {
    return uint8_t(mln::underlying_type(t));
}

constexpr const auto properties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"atmosphere-blend", toUint8(Property::AtmosphereBlend)},
     {"fog-color", toUint8(Property::FogColor)},
     {"fog-ground-blend", toUint8(Property::FogGroundBlend)},
     {"horizon-color", toUint8(Property::HorizonColor)},
     {"horizon-fog-blend", toUint8(Property::HorizonFogBlend)},
     {"sky-color", toUint8(Property::SkyColor)},
     {"sky-horizon-blend", toUint8(Property::SkyHorizonBlend)},
     {"atmosphere-blend-transition", toUint8(Property::AtmosphereBlendTransition)},
     {"fog-color-transition", toUint8(Property::FogColorTransition)},
     {"fog-ground-blend-transition", toUint8(Property::FogGroundBlendTransition)},
     {"horizon-color-transition", toUint8(Property::HorizonColorTransition)},
     {"horizon-fog-blend-transition", toUint8(Property::HorizonFogBlendTransition)},
     {"sky-color-transition", toUint8(Property::SkyColorTransition)},
     {"sky-horizon-blend-transition", toUint8(Property::SkyHorizonBlendTransition)}});

} // namespace

std::optional<Error> Sky::setProperty(const std::string& name, const Convertible& value) {
    const auto it = properties.find(name.c_str());
    if (it == properties.end()) {
        return Error{"sky doesn't support this property"};
    }

    auto property = static_cast<Property>(it->second);

    if (property == Property::AtmosphereBlend || property == Property::FogGroundBlend ||
        property == Property::HorizonFogBlend || property == Property::SkyHorizonBlend) {
        Error error;
        std::optional<PropertyValue<float>> typedValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == Property::AtmosphereBlend) {
            setAtmosphereBlend(*typedValue);
            return std::nullopt;
        }

        if (property == Property::FogGroundBlend) {
            setFogGroundBlend(*typedValue);
            return std::nullopt;
        }

        if (property == Property::HorizonFogBlend) {
            setHorizonFogBlend(*typedValue);
            return std::nullopt;
        }

        if (property == Property::SkyHorizonBlend) {
            setSkyHorizonBlend(*typedValue);
            return std::nullopt;
        }
    }

    if (property == Property::FogColor || property == Property::HorizonColor || property == Property::SkyColor) {
        Error error;
        std::optional<PropertyValue<Color>> typedValue = convert<PropertyValue<Color>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == Property::FogColor) {
            setFogColor(*typedValue);
            return std::nullopt;
        }

        if (property == Property::HorizonColor) {
            setHorizonColor(*typedValue);
            return std::nullopt;
        }

        if (property == Property::SkyColor) {
            setSkyColor(*typedValue);
            return std::nullopt;
        }
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == Property::AtmosphereBlendTransition) {
        setAtmosphereBlendTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::FogColorTransition) {
        setFogColorTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::FogGroundBlendTransition) {
        setFogGroundBlendTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::HorizonColorTransition) {
        setHorizonColorTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::HorizonFogBlendTransition) {
        setHorizonFogBlendTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::SkyColorTransition) {
        setSkyColorTransition(*transition);
        return std::nullopt;
    }

    if (property == Property::SkyHorizonBlendTransition) {
        setSkyHorizonBlendTransition(*transition);
        return std::nullopt;
    }

    return Error{"sky doesn't support this property"};
}

StyleProperty Sky::getProperty(const std::string& name) const {
    const auto it = properties.find(name.c_str());
    if (it == properties.end()) {
        return {};
    }

    switch (static_cast<Property>(it->second)) {
        case Property::AtmosphereBlend:
            return makeStyleProperty(getAtmosphereBlend());
        case Property::FogColor:
            return makeStyleProperty(getFogColor());
        case Property::FogGroundBlend:
            return makeStyleProperty(getFogGroundBlend());
        case Property::HorizonColor:
            return makeStyleProperty(getHorizonColor());
        case Property::HorizonFogBlend:
            return makeStyleProperty(getHorizonFogBlend());
        case Property::SkyColor:
            return makeStyleProperty(getSkyColor());
        case Property::SkyHorizonBlend:
            return makeStyleProperty(getSkyHorizonBlend());
        case Property::AtmosphereBlendTransition:
            return makeStyleProperty(getAtmosphereBlendTransition());
        case Property::FogColorTransition:
            return makeStyleProperty(getFogColorTransition());
        case Property::FogGroundBlendTransition:
            return makeStyleProperty(getFogGroundBlendTransition());
        case Property::HorizonColorTransition:
            return makeStyleProperty(getHorizonColorTransition());
        case Property::HorizonFogBlendTransition:
            return makeStyleProperty(getHorizonFogBlendTransition());
        case Property::SkyColorTransition:
            return makeStyleProperty(getSkyColorTransition());
        case Property::SkyHorizonBlendTransition:
            return makeStyleProperty(getSkyHorizonBlendTransition());
    }
    return {};
}

float Sky::getDefaultAtmosphereBlend() {
    return SkyAtmosphereBlend::defaultValue();
}

PropertyValue<float> Sky::getAtmosphereBlend() const {
    return impl->properties.template get<SkyAtmosphereBlend>().value;
}

void Sky::setAtmosphereBlend(PropertyValue<float> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyAtmosphereBlend>().value = std::move(property);
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

void Sky::setAtmosphereBlendTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyAtmosphereBlend>().options = options;
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

TransitionOptions Sky::getAtmosphereBlendTransition() const {
    return impl->properties.template get<SkyAtmosphereBlend>().options;
}

Color Sky::getDefaultFogColor() {
    return SkyFogColor::defaultValue();
}

PropertyValue<Color> Sky::getFogColor() const {
    return impl->properties.template get<SkyFogColor>().value;
}

void Sky::setFogColor(PropertyValue<Color> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyFogColor>().value = std::move(property);
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

void Sky::setFogColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyFogColor>().options = options;
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

TransitionOptions Sky::getFogColorTransition() const {
    return impl->properties.template get<SkyFogColor>().options;
}

float Sky::getDefaultFogGroundBlend() {
    return SkyFogGroundBlend::defaultValue();
}

PropertyValue<float> Sky::getFogGroundBlend() const {
    return impl->properties.template get<SkyFogGroundBlend>().value;
}

void Sky::setFogGroundBlend(PropertyValue<float> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyFogGroundBlend>().value = std::move(property);
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

void Sky::setFogGroundBlendTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyFogGroundBlend>().options = options;
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

TransitionOptions Sky::getFogGroundBlendTransition() const {
    return impl->properties.template get<SkyFogGroundBlend>().options;
}

Color Sky::getDefaultHorizonColor() {
    return SkyHorizonColor::defaultValue();
}

PropertyValue<Color> Sky::getHorizonColor() const {
    return impl->properties.template get<SkyHorizonColor>().value;
}

void Sky::setHorizonColor(PropertyValue<Color> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyHorizonColor>().value = std::move(property);
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

void Sky::setHorizonColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyHorizonColor>().options = options;
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

TransitionOptions Sky::getHorizonColorTransition() const {
    return impl->properties.template get<SkyHorizonColor>().options;
}

float Sky::getDefaultHorizonFogBlend() {
    return SkyHorizonFogBlend::defaultValue();
}

PropertyValue<float> Sky::getHorizonFogBlend() const {
    return impl->properties.template get<SkyHorizonFogBlend>().value;
}

void Sky::setHorizonFogBlend(PropertyValue<float> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyHorizonFogBlend>().value = std::move(property);
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

void Sky::setHorizonFogBlendTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyHorizonFogBlend>().options = options;
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

TransitionOptions Sky::getHorizonFogBlendTransition() const {
    return impl->properties.template get<SkyHorizonFogBlend>().options;
}

Color Sky::getDefaultSkyColor() {
    return SkyColor::defaultValue();
}

PropertyValue<Color> Sky::getSkyColor() const {
    return impl->properties.template get<SkyColor>().value;
}

void Sky::setSkyColor(PropertyValue<Color> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyColor>().value = std::move(property);
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

void Sky::setSkyColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyColor>().options = options;
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

TransitionOptions Sky::getSkyColorTransition() const {
    return impl->properties.template get<SkyColor>().options;
}

float Sky::getDefaultSkyHorizonBlend() {
    return SkyHorizonBlend::defaultValue();
}

PropertyValue<float> Sky::getSkyHorizonBlend() const {
    return impl->properties.template get<SkyHorizonBlend>().value;
}

void Sky::setSkyHorizonBlend(PropertyValue<float> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyHorizonBlend>().value = std::move(property);
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

void Sky::setSkyHorizonBlendTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<SkyHorizonBlend>().options = options;
    impl = std::move(impl_);
    observer->onSkyChanged(*this);
}

TransitionOptions Sky::getSkyHorizonBlendTransition() const {
    return impl->properties.template get<SkyHorizonBlend>().options;
}

} // namespace style
} // namespace mln
