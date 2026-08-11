#pragma once

#include <mbgl/plugin/plugin_registry.hpp>
#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/layer_properties.hpp>

#include <memory>

namespace mbgl {
namespace style {

class PluginStyleLayer final : public Layer {
public:
    PluginStyleLayer(const std::string& id, plugin::LayerType);
    ~PluginStyleLayer() final;

    class Impl;
    const Impl& impl() const;
    Mutable<Impl> mutableImpl() const;

private:
    explicit PluginStyleLayer(Immutable<Impl>);

    std::optional<conversion::Error> setPropertyInternal(const std::string&, const conversion::Convertible&) final;
    StyleProperty getProperty(const std::string&) const final;
    std::unique_ptr<Layer> cloneRef(const std::string& id) const final;
    Mutable<Layer::Impl> mutableBaseImpl() const final;
};

class PluginStyleLayer::Impl final : public Layer::Impl {
public:
    Impl(const std::string& id, plugin::LayerType);
    Impl(const Impl&) = default;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;
    const LayerTypeInfo* getTypeInfo() const noexcept override;
    bool isPluginStyleLayer() const noexcept override { return true; }

    struct TypeInfoHolder;
    plugin::LayerType registration;
    std::shared_ptr<const TypeInfoHolder> typeInfo;
};

class PluginStyleLayerProperties final : public LayerProperties {
public:
    explicit PluginStyleLayerProperties(Immutable<PluginStyleLayer::Impl> impl)
        : LayerProperties(std::move(impl)) {}

    expression::Dependency getDependencies() const noexcept override { return expression::Dependency::None; }
};

} // namespace style
} // namespace mbgl
