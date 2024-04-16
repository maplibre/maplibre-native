#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

/// Dynamic UBO
struct alignas(16) SymbolDynamicUBO {
    /* 0 */ float fade_change;
    /* 4 */ float camera_to_center_distance;
    /* 8 */ float aspect_ratio;
    /* 12 */ float pad;
    /* 16 */
};
static_assert(sizeof(SymbolDynamicUBO) == 16);

struct alignas(16) SymbolDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 4 * 4> label_plane_matrix;
    /* 128 */ std::array<float, 4 * 4> coord_matrix;

    /* 192 */ std::array<float, 2> texsize;
    /* 200 */ std::array<float, 2> texsize_icon;

    /* 208 */ float gamma_scale;
    /* 212 */ /*bool*/ int rotate_symbol;

    /* 216 */ std::array<float, 2> pad;
    /* 224 */
};
static_assert(sizeof(SymbolDrawableUBO) == 14 * 16);

/// Evaluated properties that depend on the tile
struct alignas(16) SymbolTilePropsUBO {
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
static_assert(sizeof(SymbolTilePropsUBO) == 2 * 16);

/// Attribute interpolations
struct alignas(16) SymbolInterpolateUBO {
    /*  0 */ float fill_color_t;
    /*  4 */ float halo_color_t;
    /*  8 */ float opacity_t;
    /* 12 */ float halo_width_t;
    /* 16 */ float halo_blur_t;
    /* 20 */ float pad1, pad2, pad3;
    /* 32 */
};
static_assert(sizeof(SymbolInterpolateUBO) == 32);

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

enum {
    idSymbolDynamicUBO,
    idSymbolDrawableUBO,
    idSymbolTilePropsUBO,
    idSymbolInterpolateUBO,
    idSymbolEvaluatedPropsUBO,
    symbolUBOCount
};

} // namespace shaders
} // namespace mbgl
