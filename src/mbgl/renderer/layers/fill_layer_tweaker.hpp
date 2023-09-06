#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string>

namespace mbgl {

/**
    Fill layer specific tweaker
 */
class FillLayerTweaker : public LayerTweaker {
public:
    FillLayerTweaker(std::string id, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id), properties) {}

public:
    ~FillLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

private:
    gfx::UniformBufferPtr fillPropsUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePropsUniformBuffer;
    gfx::UniformBufferPtr fillPatternPropsUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePatternPropsUniformBuffer;

#if MLN_RENDER_BACKEND_METAL
    gfx::UniformBufferPtr fillPermutationUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePermutationUniformBuffer;
    gfx::UniformBufferPtr fillPatternPermutationUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePatternPermutationUniformBuffer;
    gfx::UniformBufferPtr expressionUniformBuffer;
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
