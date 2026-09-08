#pragma once

#include <mln/layermanager/layer_factory.hpp>
#include <mln/plugin/plugin_registry.hpp>

namespace mln::plugin {
// C descriptors adapt ordinary C++ factories instead of adding a dispatch path.
class PluginLayerFactory final : public LayerFactory {
public:
    explicit PluginLayerFactory(LayerType registration_) : registration(std::move(registration_)) {}
    const style::LayerTypeInfo* getTypeInfo() const noexcept override;
    std::unique_ptr<style::Layer> createLayer(const std::string&, const style::conversion::Convertible&) noexcept override;
    std::unique_ptr<RenderLayer> createRenderLayer(Immutable<style::Layer::Impl>) noexcept override;
    std::unique_ptr<Layout> createLayout(const LayoutParameters&, std::unique_ptr<GeometryTileLayer>,
                                       const std::vector<Immutable<style::LayerProperties>>&) override;
private:
    const LayerType registration;
};
} // namespace mln::plugin
