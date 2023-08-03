#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

/**
    Heatmap layer specific tweaker
 */
class HeatmapLayerTweaker : public LayerTweaker {
public:
    HeatmapLayerTweaker(std::string id, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id), properties){}

public:
    ~HeatmapLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;
};

} // namespace mbgl
