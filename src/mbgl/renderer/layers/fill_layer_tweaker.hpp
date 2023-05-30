#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

/**
    Fill layer specific tweaker
 */
class FillLayerTweaker : public LayerTweaker {
public:
    FillLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~FillLayerTweaker() override = default;

    void execute(LayerGroup&, const RenderTree&, const PaintParameters&) override;
};

} // namespace mbgl
