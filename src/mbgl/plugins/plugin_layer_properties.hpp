#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/plugins/plugin_layer.hpp>
// #include <mbgl/style/layout_property.hpp>
// #include <mbgl/style/paint_property.hpp>
// #include <mbgl/style/properties.hpp>
// #include <mbgl/programs/attributes.hpp>
// #include <mbgl/programs/uniforms.hpp>

namespace mbgl {
namespace style {

class PluginLayerProperties final : public LayerProperties {
public:
    // explicit PluginLayerProperties(Immutable<PluginLayer::Impl>);
    PluginLayerProperties(Immutable<PluginLayer::Impl>);
    //    PluginLayerProperties(
    //        Immutable<PluginLayer::Impl>,
    //        HeatmapPaintProperties::PossiblyEvaluated);
    ~PluginLayerProperties() override;

    // TODO: What is this?
    unsigned long constantsMask() const override;

    // TODO: What is this?
    expression::Dependency getDependencies() const noexcept override;

    // const PluginLayerProperties::Impl& layerImpl() const noexcept;
    //  Data members.
    //    HeatmapPaintProperties::PossiblyEvaluated evaluated;
};

} // namespace style
} // namespace mbgl

// clang-format on
