#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

/**
    Circle layer specific tweaker
 */
class CircleLayerTweaker : public LayerTweaker {
public:
    CircleLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~CircleLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr paintParamsUniformBuffer;
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;
};

} // namespace mbgl
