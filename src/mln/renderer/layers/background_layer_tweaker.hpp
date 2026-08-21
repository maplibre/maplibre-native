#pragma once

#include <mln/renderer/layer_tweaker.hpp>

#include <memory>

namespace mln {

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

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif
};

} // namespace mln
