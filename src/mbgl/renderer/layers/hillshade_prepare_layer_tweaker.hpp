#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

/**
    Hillshade prepare layer specific tweaker
 */
class HillshadePrepareLayerTweaker : public LayerTweaker {
public:
    HillshadePrepareLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~HillshadePrepareLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

protected:
};

} // namespace mbgl
