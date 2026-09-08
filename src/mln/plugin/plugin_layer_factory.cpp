#include <mln/plugin/plugin_layer_factory.hpp>
#include <mln/layout/plugin_layout.hpp>
#include <mln/renderer/layers/render_plugin_style_layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/tile/geometry_tile_data.hpp>

namespace mln::plugin {
const style::LayerTypeInfo* PluginLayerFactory::getTypeInfo() const noexcept {
    return &registration.identity->info;
}

std::unique_ptr<style::Layer> PluginLayerFactory::createLayer(
    const std::string& id, const style::conversion::Convertible& value) noexcept {
    const auto source = getSource(value);
    if (!source || source->empty()) return nullptr;
    return std::make_unique<style::PluginStyleLayer>(id, *source, registration);
}

std::unique_ptr<RenderLayer> PluginLayerFactory::createRenderLayer(Immutable<style::Layer::Impl> impl) noexcept {
    return std::make_unique<RenderPluginStyleLayer>(staticImmutableCast<style::PluginStyleLayer::Impl>(std::move(impl)));
}

std::unique_ptr<Layout> PluginLayerFactory::createLayout(
    const LayoutParameters& parameters, std::unique_ptr<GeometryTileLayer> tileLayer,
    const std::vector<Immutable<style::LayerProperties>>& layers) {
    return std::make_unique<PluginLayout>(parameters.bucketParameters, layers, std::move(tileLayer), registration);
}
} // namespace mln::plugin
