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
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 4 * 4> label_plane_matrix;
    /* 128 */ std::array<float, 4 * 4> coord_matrix;
    
    /* 192 */ std::array<float, 2> texsize;
    /* 200 */ std::array<float, 2> texsize_icon;
    
    /* 208 */ float gamma_scale;
    /* 212 */ float device_pixel_ratio;
    
    /* 216 */ float camera_to_center_distance;
    /* 220 */ float pitch;
    /* 224 */ /*bool*/ int rotate_symbol;
    /* 228 */ float aspect_ratio;
    /* 232 */ float fade_change;
    /* 236 */ float pad;
    /* 240 */
};
static_assert(sizeof(SymbolDrawableUBO) == 15 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) SymbolDrawablePaintUBO {
    /*  0 */ std::array<float, 4> fill_color;
    /* 16 */ std::array<float, 4> halo_color;
    /* 32 */ float opacity;
    /* 36 */ float halo_width;
    /* 40 */ float halo_blur;
    /* 44 */ float padding;
    /* 48 */
};
static_assert(sizeof(SymbolDrawablePaintUBO) == 3 * 16);

struct alignas(16) SymbolPermutationUBO {
    /*  0 */ Attribute fill_color;
    /*  8 */ Attribute halo_color;
    /* 16 */ Attribute opacity;
    /* 24 */ Attribute halo_width;
    /* 40 */ Attribute halo_blur;
    /* 48 */ Attribute padding;
    /* 56 */ int32_t /*bool*/ overdrawInspector;
    /* 60 */ float pad;
    /* 64 */
};
static_assert(sizeof(SymbolPermutationUBO) == 4 * 16);

} // namespace shaders
} // namespace mbgl
