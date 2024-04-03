#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

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
struct alignas(16) SymbolPaintUBO {
    /*  0 */ Color fill_color;
    /* 16 */ Color halo_color;
    /* 32 */ float opacity;
    /* 36 */ float halo_width;
    /* 40 */ float halo_blur;
    /* 44 */ float padding;
    /* 48 */
};
static_assert(sizeof(SymbolPaintUBO) == 3 * 16);

enum {
    idSymbolUBOIndex,
    idSymbolDrawableUBO,
    idSymbolDynamicUBO,
    idSymbolPaintUBO,
    idSymbolTilePropsUBO,
    idSymbolInterpolateUBO,
    symbolUBOCount
};

/// Compute UBO
struct alignas(16) SymbolComputeUBO {
    /*  64 */ std::array<float, 4 * 4> projMatrix;

    /*   4 */ int32_t layerIndex;
    /*   4 */ int32_t subLayerIndex;
    /*   4 */ uint32_t width;
    /*   4 */ uint32_t height;

    /*   8 */ std::array<float, 2> texsize;
    /*   8 */ std::array<float, 2> texsize_icon;
    
    /*   4 */ uint32_t tileIdCanonicalX;
    /*   4 */ uint32_t tileIdCanonicalY;
    /*   2 */ uint16_t tileIdCanonicalZ;
    /*   2 */ int16_t tileIdWrap;
    /*   4 */ float camDist;
    
    /*   4 */ float scale;
    /*   4 */ float bearing;
    /*   4 */ float zoom;
    /*   4 */ float pitch;
    
    /*   8 */ std::array<float, 2> translation;
    
    /*   1 */ bool isAnchorMap;
    /*   1 */ bool inViewportPixelUnits;
    /*   1 */ bool pitchWithMap;
    /*   1 */ bool rotateWithMap;
    /*   1 */ bool alongLine;
    /*   1 */ bool hasVariablePlacement;
    
    /*   2 */ int16_t padding;
};
static_assert(sizeof(SymbolComputeUBO) == 9 * 16);

} // namespace shaders
} // namespace mbgl
