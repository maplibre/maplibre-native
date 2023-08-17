#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

/**
    Heatmap layer specific tweaker
 */
class HeatmapLayerTweaker : public LayerTweaker {
public:
    HeatmapLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~HeatmapLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;
};

} // namespace mbgl
