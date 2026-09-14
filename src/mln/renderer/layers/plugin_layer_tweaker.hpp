#pragma once

#include <mln/plugin/plugin_registry.hpp>
#include <mln/gfx/uniform_buffer.hpp>
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
    struct SharedUniform {
        std::vector<uint8_t> scratch;
        std::vector<uint8_t> uploaded;
        gfx::UniformBufferPtr buffer;
        // Borrowed only during execute; cleared before returning to the renderer.
        std::vector<gfx::Drawable*> drawables;
        uint32_t bindingID = 0;
        bool failed = false;
    };
    std::map<std::pair<std::string, uint32_t>, SharedUniform> sharedUniforms;
};

} // namespace mln
