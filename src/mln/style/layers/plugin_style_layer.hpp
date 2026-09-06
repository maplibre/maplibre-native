#pragma once

#include <mln/plugin/plugin_registry.hpp>
#include <mln/style/layer_impl.hpp>
#include <mln/style/layer_properties.hpp>

#include <memory>

namespace mln {
namespace style {

class PluginStyleLayer final : public Layer {
public:
    PluginStyleLayer(const std::string& id, const std::string& source, plugin::LayerType);
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
    Impl(const std::string& id, const std::string& source, plugin::LayerType);
    Impl(const Impl&) = default;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;
    const LayerTypeInfo* getTypeInfo() const noexcept override;
    expression::Dependency getDependencies() const noexcept override;
    bool isPluginStyleLayer() const noexcept override { return true; }

    struct TypeInfoHolder;
    plugin::LayerType registration;
    std::shared_ptr<const TypeInfoHolder> typeInfo;
};

class PluginStyleLayerProperties final : public LayerProperties {
public:
    explicit PluginStyleLayerProperties(Immutable<PluginStyleLayer::Impl> impl,
                                        PluginPropertyMap evaluatedPaintProperties_ = {})
        : LayerProperties(std::move(impl)),
          evaluatedPaintProperties(std::move(evaluatedPaintProperties_)) {
        const auto& registration = static_cast<const PluginStyleLayer::Impl&>(*baseImpl).registration;
        switch (registration.renderStage) {
            case MLN_PLUGIN_RENDER_STAGE_PASS_3D:
                renderPasses = 1u << 2u;
                break;
            case MLN_PLUGIN_RENDER_STAGE_OPAQUE:
                renderPasses = 1u << 0u;
                break;
            case MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT:
                renderPasses = 1u << 1u;
                break;
            default:
                renderPasses = 0;
                break;
        }
        if (registration.participatesIn3DPass) renderPasses |= 1u << 2u;
    }

    expression::Dependency getDependencies() const noexcept override { return baseImpl->getDependencies(); }

    PluginPropertyMap evaluatedPaintProperties;
};

} // namespace style
} // namespace mln
