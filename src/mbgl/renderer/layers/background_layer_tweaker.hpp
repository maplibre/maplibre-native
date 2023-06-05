#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <memory>

namespace mbgl {

namespace gfx {
class ShaderProgramBase;
class UniformBuffer;

using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;
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
};

} // namespace mbgl
