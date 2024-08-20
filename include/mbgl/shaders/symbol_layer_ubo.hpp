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

    /* 208 */ float gamma_scale;
    /* 212 */ /*bool*/ int rotate_symbol;

    /* 216 */ std::array<float, 2> pad;
    /* 224 */
};
static_assert(sizeof(SymbolDrawableUBO) == 14 * 16);

enum class SymbolTileFlags : uint32_t {
    None = 0,
    IsText = 1 << 0,
    DoFill = 1 << 1,
    DoHalo = 1 << 2,
    PitchWithMap = 1 << 3,
    IsSizeZoomConstant = 1 << 4,
    IsSizeFeatureConstant = 1 << 5,
};

/// Evaluated properties that depend on the tile
struct alignas(16) SymbolTilePropsUBO {
    /*  0 */ SymbolTileFlags flags;
    /*  4 */ float size_t;
    /*  8 */ float size;
    /* 12 */ float padding;
    /* 16 */
};
static_assert(sizeof(SymbolTilePropsUBO) == 1 * 16);

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
    idSymbolDrawableUBO = globalUBOCount,
    idSymbolTilePropsUBO,
    idSymbolInterpolateUBO,
    idSymbolEvaluatedPropsUBO,
    symbolUBOCount
};

} // namespace shaders
} // namespace mbgl
