#pragma once

#include "plugin_layer.hpp"
#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/layer_properties.hpp>

#include <memory>

namespace mbgl {

class TransformState;

namespace style {

class PluginLayer::Impl : public Layer::Impl {
public:
    Impl(const std::string& id, std::unique_ptr<PluginLayerHost> host);

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    std::shared_ptr<PluginLayerHost> host;

    DECLARE_LAYER_TYPE_INFO;
};

class PluginLayerProperties : public LayerProperties {
public:
    explicit PluginLayerProperties(Immutable<CustomLayer::Impl> impl)
        : LayerProperties(std::move(impl)) {}

    expression::Dependency getDependencies() const noexcept override { return expression::Dependency::None; }
};

} // namespace style
} // namespace mbgl
