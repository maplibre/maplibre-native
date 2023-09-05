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

    static constexpr std::string_view SymbolDrawableUBOName = "SymbolDrawableUBO";
    static constexpr std::string_view SymbolDynamicUBOName = "SymbolDynamicUBO";
    static constexpr std::string_view SymbolDrawablePaintUBOName = "SymbolDrawablePaintUBO";
    static constexpr std::string_view SymbolDrawableTilePropsUBOName = "SymbolDrawableTilePropsUBO";
    static constexpr std::string_view SymbolDrawableInterpolateUBOName = "SymbolDrawableInterpolateUBO";

private:
    gfx::UniformBufferPtr textPaintBuffer;
    gfx::UniformBufferPtr iconPaintBuffer;
};

/// Evaluated properties that depend on the tile
struct alignas(16) SymbolDrawableTilePropsUBO {
    /*  0 */ /*bool*/ int is_text;
    /*  4 */ /*bool*/ int is_halo;
    /*  8 */ /*bool*/ int pitch_with_map;
    /* 12 */ /*bool*/ int is_size_zoom_constant;
    /* 16 */ /*bool*/ int is_size_feature_constant;
    /* 20 */ float size_t;
    /* 24 */ float size;
    /* 28 */ float padding;
    /* 32 */
};
static_assert(sizeof(SymbolDrawableTilePropsUBO) == 2 * 16);

/// Attribute interpolations
struct alignas(16) SymbolDrawableInterpolateUBO {
    /*  0 */ float fill_color_t;
    /*  4 */ float halo_color_t;
    /*  8 */ float opacity_t;
    /* 12 */ float halo_width_t;
    /* 16 */ float halo_blur_t;
    /* 20 */ float pad1, pad2, pad3;
    /* 32 */
};
static_assert(sizeof(SymbolDrawableInterpolateUBO) == 32);

} // namespace mbgl
