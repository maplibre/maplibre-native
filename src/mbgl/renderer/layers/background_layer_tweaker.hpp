#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

namespace gfx {
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

/**
    Background layer specific tweaker
 */
class BackgroundLayerTweaker : public LayerTweaker {
public:
    BackgroundLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~BackgroundLayerTweaker() override = default;

    void execute(LayerGroup&, const RenderTree&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr layerUniformBuffer = nullptr;
};

} // namespace mbgl
