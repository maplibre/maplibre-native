#pragma once

#include <mln/plugin/plugin_registry.hpp>
#include <mln/renderer/layer_tweaker.hpp>

#include <map>

namespace mln {

class PluginLayerTweaker final : public LayerTweaker {
public:
    PluginLayerTweaker(std::string id, Immutable<style::LayerProperties> properties, plugin::LayerType registration_)
        : LayerTweaker(std::move(id), std::move(properties)),
          registration(std::move(registration_)) {}

    void execute(LayerGroupBase&, const PaintParameters&) override;

private:
    const plugin::LayerType registration;
};

} // namespace mln
