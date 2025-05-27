#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/plugin/plugin_layer.hpp>
// #include <mbgl/style/layout_property.hpp>
// #include <mbgl/style/paint_property.hpp>
// #include <mbgl/style/properties.hpp>
// #include <mbgl/programs/attributes.hpp>
// #include <mbgl/programs/uniforms.hpp>

namespace mbgl {
namespace style {

// TODO: Maybe remove

class PluginLayerProperties final : public LayerProperties {
public:
    PluginLayerProperties(Immutable<PluginLayer::Impl>);
    ~PluginLayerProperties() override;

    // TODO: What is this?
    unsigned long constantsMask() const override;

    // TODO: What is this?
    expression::Dependency getDependencies() const noexcept override;

};

} // namespace style
} // namespace mbgl

// clang-format on
