#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string>

namespace mbgl {

/**
    Symbol layer specific tweaker
 */
class SymbolLayerTweaker : public LayerTweaker {
public:
    SymbolLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

public:
    ~SymbolLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

private:
    gfx::UniformBufferPtr textPaintBuffer;
    gfx::UniformBufferPtr iconPaintBuffer;
    gfx::UniformBufferPtr dynamicBuffer;
    gfx::UniformBufferPtr drawableBuffer;

    bool textPropertiesUpdated = false;
    bool iconPropertiesUpdated = false;

#if MLN_RENDER_BACKEND_METAL
    gfx::UniformBufferPtr permutationUniformBuffer;
    gfx::UniformBufferPtr expressionUniformBuffer;
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
