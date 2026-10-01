
#include <mln/plugin/plugin_layer_properties.hpp>
#include <mln/plugin/plugin_layer_impl.hpp>

namespace mln {
namespace style {

PluginLayerProperties::PluginLayerProperties(Immutable<PluginLayer::Impl> impl_)
    : LayerProperties(std::move(impl_)) {}

PluginLayerProperties::~PluginLayerProperties() = default;

unsigned long PluginLayerProperties::constantsMask() const {
    return 0;
}

expression::Dependency PluginLayerProperties::getDependencies() const noexcept {
    return expression::Dependency::None;
}

} // namespace style
} // namespace mln
