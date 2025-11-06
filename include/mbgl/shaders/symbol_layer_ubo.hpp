#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) SymbolDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 4 * 4> label_plane_matrix;
    /* 128 */ std::array<float, 4 * 4> coord_matrix;

    /* 192 */ std::array<float, 2> texsize;
    /* 200 */ std::array<float, 2> texsize_icon;

    /* 208 */ /*bool*/ int is_text_prop;
    /* 212 */ /*bool*/ int rotate_symbol;
    /* 216 */ /*bool*/ int pitch_with_map;
    /* 220 */ /*bool*/ int is_size_zoom_constant;
    /* 224 */ /*bool*/ int is_size_feature_constant;
    /* 228 */ /*bool*/ int is_offset;

    /* 232 */ float size_t;
    /* 236 */ float size;

    // Interpolations
    /* 240 */ float fill_color_t;
    /* 244 */ float halo_color_t;
    /* 248 */ float opacity_t;
    /* 252 */ float halo_width_t;
    /* 256 */ float halo_blur_t;
    /* 260 */
};
static_assert(sizeof(SymbolDrawableUBO) == 17 * 16);

struct alignas(16) SymbolTilePropsUBO {
    /*  0 */ /*bool*/ int is_text;
    /*  4 */ /*bool*/ int is_halo;
    /*  8 */ float gamma_scale;
    /* 12 */ float pad1;
    /* 16 */
};
static_assert(sizeof(SymbolTilePropsUBO) == 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) SymbolEvaluatedPropsUBO {
    /*  0 */ Color text_fill_color;
    /* 16 */ Color text_halo_color;
    /* 32 */ float text_opacity;
    /* 36 */ float text_halo_width;
    /* 40 */ float text_halo_blur;
    /* 44 */ float pad1;
    /* 48 */ Color icon_fill_color;
    /* 64 */ Color icon_halo_color;
    /* 80 */ float icon_opacity;
    /* 84 */ float icon_halo_width;
    /* 88 */ float icon_halo_blur;
    /* 92 */ float pad2;
    /* 96 */
};
static_assert(sizeof(SymbolEvaluatedPropsUBO) == 6 * 16);

} // namespace shaders
} // namespace mbgl
