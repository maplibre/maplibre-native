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
    BackgroundLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

public:
    ~BackgroundLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr backgroundLayerBuffer;
};

} // namespace mbgl
