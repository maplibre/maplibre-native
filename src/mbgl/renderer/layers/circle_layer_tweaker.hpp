#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

namespace gfx {
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

/**
    Circle layer specific tweaker
 */
class CircleLayerTweaker : public LayerTweaker {
public:
    CircleLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~CircleLayerTweaker() override = default;

    void execute(LayerGroup&, const RenderTree&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr paintParamsUniformBuffer = nullptr;
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer = nullptr;
    gfx::UniformBufferPtr interpolateUniformBuffer = nullptr;
};

} // namespace mbgl
