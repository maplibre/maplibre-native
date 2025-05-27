//
//  plugin_layer_properties.cpp
//  App
//
//  Created by Malcolm Toon on 4/25/25.
//

#include "plugin_layer_properties.hpp"
#include <mbgl/plugin/plugin_layer_impl.hpp>
#include "plugin_layer_debug.hpp"

namespace mbgl {
namespace style {

PluginLayerProperties::PluginLayerProperties(Immutable<PluginLayer::Impl> impl_)
    : LayerProperties(std::move(impl_)) {}

PluginLayerProperties::~PluginLayerProperties() = default;

unsigned long PluginLayerProperties::constantsMask() const {
    // TODO: What are these and how should they be implemented for plugins?
    return 0; // evaluated.constantsMask();
}

expression::Dependency PluginLayerProperties::getDependencies() const noexcept {
    // TODO: What are dependencies and how should they be implemented in the plugin paradigm
    return expression::Dependency::None;
    // return layerImpl().paint.getDependencies();
}

} // namespace style
} // namespace mbgl
