#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

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

struct alignas(16) SymbolDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> label_plane_matrix;
    /*  64 */ std::array<float, 4 * 4> coord_matrix;

    /* 128 */ std::array<float, 2> texsize;
    /* 136 */ std::array<float, 2> texsize_icon;

    /* 144 */ float gamma_scale;
    /* 148 */ /*bool*/ int rotate_symbol;

    /* 152 */ std::array<float, 2> pad;
    /* 160 */
};
static_assert(sizeof(SymbolDrawableUBO) == 10 * 16);

/// Dynamic UBO
struct alignas(16) SymbolDynamicUBO {
    /* 0 */ float fade_change;
    /* 4 */ float camera_to_center_distance;
    /* 8 */ float aspect_ratio;
    /* 12 */ float pad;
    /* 16 */
};
static_assert(sizeof(SymbolDynamicUBO) == 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) SymbolDrawablePaintUBO {
    /*  0 */ Color fill_color;
    /* 16 */ Color halo_color;
    /* 32 */ float opacity;
    /* 36 */ float halo_width;
    /* 40 */ float halo_blur;
    /* 44 */ float padding;
    /* 48 */
};
static_assert(sizeof(SymbolDrawablePaintUBO) == 3 * 16);

} // namespace shaders
} // namespace mbgl
