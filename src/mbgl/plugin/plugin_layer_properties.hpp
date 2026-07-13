#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/plugin/plugin_layer.hpp>

namespace mbgl {
namespace style {

class PluginLayerProperties final : public LayerProperties {
public:
    PluginLayerProperties(Immutable<PluginLayer::Impl>);
    ~PluginLayerProperties() override;

    unsigned long constantsMask() const override;

    expression::Dependency getDependencies() const noexcept override;
};

} // namespace style
} // namespace mbgl
