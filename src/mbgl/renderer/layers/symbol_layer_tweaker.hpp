#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string>

namespace mbgl {

/**
    Symbol layer specific tweaker
 */
class SymbolLayerTweaker : public LayerTweaker {
public:
    SymbolLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~SymbolLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

    static constexpr std::string_view SymbolTilePropsUBOName = "SymbolDrawableTilePropsUBO";
    static constexpr std::string_view SymbolInterpolateUBOName = "SymbolInterpolateUBO";

private:
    gfx::UniformBufferPtr propsBuffer;
};

/// Evaluated properties that depend on the tile
struct alignas(16) SymbolDrawableTilePropsUBO {
    /*  0 */ std::array<float, 4> pattern_from;
    /* 16 */ std::array<float, 4> pattern_to;
    /* 32 */
};
static_assert(sizeof(SymbolDrawableTilePropsUBO) == 32);

/// Attribute interpolations
struct alignas(16) SymbolInterpolateUBO {
    /*  0 */ float color_t;
    /*  4 */ float opacity_t;
    /*  8 */ float outline_color_t;
    /* 12 */ float pattern_from_t;
    /* 16 */ float pattern_to_t;
    /* 20 */ float fade;
    /* 24 */ std::array<float, 2> padding;
    /* 32 */
};
static_assert(sizeof(SymbolInterpolateUBO) == 32);

} // namespace mbgl
