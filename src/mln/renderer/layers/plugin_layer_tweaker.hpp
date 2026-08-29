#pragma once

#include <mln/renderer/layer_tweaker.hpp>

namespace mln {

class PluginLayerTweaker final : public LayerTweaker {
public:
    PluginLayerTweaker(std::string id, Immutable<style::LayerProperties> properties, bool requires3D_)
        : LayerTweaker(std::move(id), std::move(properties)),
          requires3D(requires3D_) {}

    void execute(LayerGroupBase&, const PaintParameters&) override;

private:
    const bool requires3D;
#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif
};

} // namespace mln
