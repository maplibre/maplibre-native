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

    static constexpr std::string_view SymbolDrawableTilePropsUBOName = "SymbolDrawableTilePropsUBO";
    static constexpr std::string_view SymbolDrawableInterpolateUBOName = "SymbolDrawableInterpolateUBO";

private:
    gfx::UniformBufferPtr textBuffer;
    gfx::UniformBufferPtr iconBuffer;
};

/// Evaluated properties that depend on the tile
struct alignas(16) SymbolDrawableTilePropsUBO {
    /*  0 */ bool is_text;
    /*  1 */ bool is_halo;
    /*  2 */ bool pitch_with_map;
    /*  3 */ bool is_size_zoom_constant;
    /*  4 */ bool is_size_feature_constant;
    /*  8 */ float size_t;
    /* 12 */ float size;
    /* 16 */
};
static_assert(sizeof(SymbolDrawableTilePropsUBO) == 16);

/// Attribute interpolations
struct alignas(16) SymbolDrawableInterpolateUBO {
    /*  0 */ float fill_color_t;
    /*  4 */ float halo_color_t;
    /*  8 */ float opacity_t;
    /* 12 */ float halo_width_t;
    /* 16 */ float halo_blur_t;
    /* 20 */ std::array<float, 3> padding;
    /* 32 */
};
static_assert(sizeof(SymbolDrawableInterpolateUBO) == 32);

} // namespace mbgl
