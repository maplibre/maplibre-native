#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

/**
    Hillshade layer specific tweaker
 */
class HillshadeLayerTweaker : public LayerTweaker {
public:
    HillshadeLayerTweaker(std::string id, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id), properties) {}

public:
    ~HillshadeLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;
};

} // namespace mbgl
