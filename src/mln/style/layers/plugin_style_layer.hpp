#pragma once

#include <mln/plugin/plugin_registry.hpp>
#include <mln/style/layer_impl.hpp>
#include <mln/style/plugin_property.hpp>
#include <mln/style/layer_properties.hpp>

#include <memory>

namespace mln {
namespace style {

class PluginStyleLayer final : public Layer {
public:
    PluginStyleLayer(const std::string& id, const std::string& source, plugin::LayerType);
    ~PluginStyleLayer() final;

    using Layer::setProperty;
    std::optional<conversion::Error> setProperty(const std::string&,
                                                 const conversion::Convertible&,
                                                 PropertyScope) final;
    Value serialize() const final;
    StyleProperty getProperty(const std::string&) const final;

    class Impl;
    const Impl& impl() const;
    Mutable<Impl> mutableImpl() const;

private:
    explicit PluginStyleLayer(Immutable<Impl>);
    std::optional<conversion::Error> setPluginProperty(const std::string&,
                                                       const conversion::Convertible&,
                                                       std::optional<PropertyScope> = std::nullopt);
    std::optional<conversion::Error> setPluginTransition(const std::string&,
                                                         const conversion::Convertible&,
                                                         std::optional<PropertyScope> = std::nullopt);

    std::optional<conversion::Error> setPropertyInternal(const std::string&, const conversion::Convertible&) final;
    std::unique_ptr<Layer> cloneRef(const std::string& id) const final;
    Mutable<Layer::Impl> mutableBaseImpl() const final;
};

class PluginStyleLayer::Impl final : public Layer::Impl {
public:
    Impl(const std::string& id, const std::string& source, plugin::LayerType);
    Impl(const Impl&) = default;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;
    const LayerTypeInfo* getTypeInfo() const noexcept override;
    expression::Dependency getDependencies() const noexcept override;

    std::map<std::string, PluginPropertyValue> pluginProperties;
    std::map<std::string, TransitionOptions> pluginPropertyTransitions;
    plugin::LayerType registration;
};

class PluginStyleLayerProperties final : public LayerProperties {
public:
    explicit PluginStyleLayerProperties(Immutable<PluginStyleLayer::Impl> impl,
                                        PluginPropertyMap evaluatedPaintProperties_ = {})
        : LayerProperties(std::move(impl)),
          evaluatedPaintProperties(std::move(evaluatedPaintProperties_)) {
        renderPasses = 1u << 1u; // Translucent pass.
    }

    expression::Dependency getDependencies() const noexcept override { return baseImpl->getDependencies(); }

    PluginPropertyMap evaluatedPaintProperties;
};

} // namespace style
} // namespace mln
