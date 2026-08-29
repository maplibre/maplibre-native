#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

class PluginLayerTweaker final : public LayerTweaker {
public:
    PluginLayerTweaker(std::string id, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id), std::move(properties)) {}

    void execute(LayerGroupBase&, const PaintParameters&) override;

private:
#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif
};

} // namespace mbgl
