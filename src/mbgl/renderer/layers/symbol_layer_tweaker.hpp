#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string>

namespace mbgl {

/**
    Symbol layer specific tweaker
 */
class SymbolLayerTweaker : public LayerTweaker {
public:
    SymbolLayerTweaker(std::string id, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id), properties) {}

public:
    ~SymbolLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

    static constexpr std::string_view SymbolDrawableUBOName = "SymbolDrawableUBO";
    static constexpr std::string_view SymbolDrawablePaintUBOName = "SymbolDrawablePaintUBO";
    static constexpr std::string_view SymbolDrawableTilePropsUBOName = "SymbolDrawableTilePropsUBO";
    static constexpr std::string_view SymbolDrawableInterpolateUBOName = "SymbolDrawableInterpolateUBO";

private:
    gfx::UniformBufferPtr textPaintBuffer;
    gfx::UniformBufferPtr iconPaintBuffer;

#if MLN_RENDER_BACKEND_METAL
    gfx::UniformBufferPtr permutationUniformBuffer;
    gfx::UniformBufferPtr expressionUniformBuffer;
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
