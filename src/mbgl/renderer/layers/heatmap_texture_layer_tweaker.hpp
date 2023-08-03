#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

/**
    Heatmap texture layer specific tweaker
 */
class HeatmapTextureLayerTweaker : public LayerTweaker {
public:
    HeatmapTextureLayerTweaker(std::string id, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id), properties) {}

public:
    ~HeatmapTextureLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

protected:
};

} // namespace mbgl
