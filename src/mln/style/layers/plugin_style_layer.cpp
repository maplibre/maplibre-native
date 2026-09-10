#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/style/layer_observer.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/conversion/transition_options.hpp>

namespace mln {
namespace style {
namespace {
constexpr const char* transitionSuffix = "-transition";

std::optional<std::string> pluginTransitionPropertyName(const char* layerType, const std::string& name) {
    constexpr std::size_t suffixLength = 11;
    if (name.size() <= suffixLength || name.compare(name.size() - suffixLength, suffixLength, transitionSuffix) != 0) {
        return std::nullopt;
    }
    auto propertyName = name.substr(0, name.size() - suffixLength);
    const auto definition = plugin::PluginRegistry::get().findProperty(layerType, propertyName);
    return definition && definition->supportsTransitions ? std::optional<std::string>{std::move(propertyName)}
                                                         : std::nullopt;
}
} // namespace

PluginStyleLayer::Impl::Impl(const std::string& id, const std::string& source, plugin::LayerType registration_)
    : Layer::Impl(id, source),
      registration(std::move(registration_)) {}

bool PluginStyleLayer::Impl::hasLayoutDifference(const Layer::Impl& other) const {
    assert(other.getTypeInfo() == getTypeInfo());
    return filter != other.filter || visibility != other.visibility;
}

void PluginStyleLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>& writer) const {
    writer.StartObject();
    writer.EndObject();
}

const LayerTypeInfo* PluginStyleLayer::Impl::getTypeInfo() const noexcept {
    return &registration.identity->info;
}

expression::Dependency PluginStyleLayer::Impl::getDependencies() const noexcept {
    expression::Dependency result = expression::Dependency::None;
    for (const auto& [name, value] : pluginProperties) {
        (void)name;
        result |= value.getDependencies();
    }
    return result;
}

PluginStyleLayer::PluginStyleLayer(const std::string& id, const std::string& source, plugin::LayerType registration)
    : Layer(makeMutable<Impl>(id, source, std::move(registration))) {}

PluginStyleLayer::PluginStyleLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {}

PluginStyleLayer::~PluginStyleLayer() = default;

const PluginStyleLayer::Impl& PluginStyleLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<PluginStyleLayer::Impl> PluginStyleLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

StyleProperty PluginStyleLayer::getProperty(const std::string& name) const {
    if (const auto propertyName = pluginTransitionPropertyName(getTypeInfo()->type, name)) {
        const auto transition = impl().pluginPropertyTransitions.find(*propertyName);
        return transition == impl().pluginPropertyTransitions.end()
                   ? StyleProperty{TransitionOptions{}.serialize(), StyleProperty::Kind::Transition}
                   : StyleProperty{transition->second.serialize(), StyleProperty::Kind::Transition};
    }
    const auto value = impl().pluginProperties.find(name);
    if (value != impl().pluginProperties.end()) {
        return value->second.toStyleProperty();
    }
    const auto definition = plugin::PluginRegistry::get().findProperty(getTypeInfo()->type, name);
    return definition ? defaultPluginPropertyValue(*definition).toStyleProperty() : StyleProperty{};
}

std::optional<conversion::Error> PluginStyleLayer::setPluginProperty(const std::string& name,
                                                                     const conversion::Convertible& value,
                                                                     std::optional<PropertyScope> scope) {
    using namespace conversion;
    const auto definition = plugin::PluginRegistry::get().findProperty(getTypeInfo()->type, name);
    if (!definition) return Error{"layer doesn't support this property"};
    if (scope) {
        if (*scope != PropertyScope::Paint) {
            return Error{"plugin property '" + name + "' is in the wrong style section"};
        }
    }

    auto impl_ = mutableImpl();
    if (isUndefined(value)) {
        if (impl_->pluginProperties.erase(name) != 0) {
            baseImpl = std::move(impl_);
            observer->onLayerChanged(*this);
        }
        return std::nullopt;
    }

    Error conversionError;
    auto converted = convertPluginPropertyValue(*definition, value, conversionError);
    if (!converted) return conversionError;
    const auto existing = impl_->pluginProperties.find(name);
    if (existing != impl_->pluginProperties.end() && existing->second == *converted) return std::nullopt;
    impl_->pluginProperties.insert_or_assign(name, std::move(*converted));
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
    return std::nullopt;
}

std::optional<conversion::Error> PluginStyleLayer::setPluginTransition(const std::string& name,
                                                                       const conversion::Convertible& value,
                                                                       std::optional<PropertyScope> scope) {
    using namespace conversion;
    const auto propertyName = pluginTransitionPropertyName(getTypeInfo()->type, name);
    if (!propertyName) return Error{"layer doesn't support this transition"};
    if (scope && *scope != PropertyScope::Paint) {
        return Error{"plugin transition '" + name + "' is in the wrong style section"};
    }

    auto impl_ = mutableImpl();
    if (isUndefined(value)) {
        if (impl_->pluginPropertyTransitions.erase(*propertyName) != 0) {
            baseImpl = std::move(impl_);
            observer->onLayerChanged(*this);
        }
        return std::nullopt;
    }

    Error error;
    auto transition = convert<TransitionOptions>(value, error);
    if (!transition) return error;
    const auto existing = impl_->pluginPropertyTransitions.find(*propertyName);
    if (existing != impl_->pluginPropertyTransitions.end() && existing->second.serialize() == transition->serialize()) {
        return std::nullopt;
    }
    impl_->pluginPropertyTransitions.insert_or_assign(*propertyName, std::move(*transition));
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
    return std::nullopt;
}

Value PluginStyleLayer::serialize() const {
    Value serialized = Layer::serialize();
    auto& result = *serialized.getObject();
    for (const auto& [name, value] : impl().pluginProperties) {
        const auto definition = plugin::PluginRegistry::get().findProperty(getTypeInfo()->type, name);
        if (!definition) continue;
        auto& object = result["paint"];
        if (!object.getObject()) object = mapbox::base::ValueObject{};
        const auto property = value.toStyleProperty();
        if (property.getKind() != StyleProperty::Kind::Undefined) {
            object.getObject()->insert_or_assign(name, property.getValue());
        }
    }
    for (const auto& [name, transition] : impl().pluginPropertyTransitions) {
        auto& paint = result["paint"];
        if (!paint.getObject()) paint = mapbox::base::ValueObject{};
        paint.getObject()->insert_or_assign(name + transitionSuffix, transition.serialize());
    }

    return serialized;
}

std::optional<conversion::Error> PluginStyleLayer::setPropertyInternal(const std::string& name,
                                                                       const conversion::Convertible& value) {
    if (pluginTransitionPropertyName(getTypeInfo()->type, name)) return setPluginTransition(name, value);
    return setPluginProperty(name, value);
}

std::optional<conversion::Error> PluginStyleLayer::setProperty(const std::string& name,
                                                               const conversion::Convertible& value,
                                                               PropertyScope scope) {
    if (pluginTransitionPropertyName(getTypeInfo()->type, name)) return setPluginTransition(name, value, scope);
    if (plugin::PluginRegistry::get().findProperty(getTypeInfo()->type, name))
        return setPluginProperty(name, value, scope);
    return Layer::setProperty(name, value);
}

std::unique_ptr<Layer> PluginStyleLayer::cloneRef(const std::string& id) const {
    auto copy = mutableImpl();
    copy->id = id;
    return std::unique_ptr<PluginStyleLayer>(new PluginStyleLayer(std::move(copy)));
}

Mutable<Layer::Impl> PluginStyleLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mln
