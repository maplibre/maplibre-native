#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) LineUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> units_to_pixels;
    float ratio;
    float device_pixel_ratio;
};
static_assert(sizeof(LineUBO) % 16 == 0);

using LineGradientUBO = LineUBO;

struct alignas(16) LinePropertiesUBO {
    Color color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float pad1, pad2, pad3;
};
static_assert(sizeof(LinePropertiesUBO) % 16 == 0);

struct alignas(16) LineGradientPropertiesUBO {
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float pad1, pad2, pad3;
};
static_assert(sizeof(LineGradientPropertiesUBO) % 16 == 0);

struct alignas(16) LinePatternUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 4> scale;
    std::array<float, 2> texsize;
    std::array<float, 2> units_to_pixels;
    float ratio;
    float device_pixel_ratio;
    float fade;
    float pad1;
};
static_assert(sizeof(LinePatternUBO) % 16 == 0);

struct alignas(16) LinePatternPropertiesUBO {
    float blur;
    float opacity;
    float offset;
    float gapwidth;
    float width;
    float pad1, pad2, pad3;
};
static_assert(sizeof(LinePatternPropertiesUBO) % 16 == 0);

struct alignas(16) LineSDFUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> units_to_pixels;
    std::array<float, 2> patternscale_a;
    std::array<float, 2> patternscale_b;
    float ratio;
    float device_pixel_ratio;
    float tex_y_a;
    float tex_y_b;
    float sdfgamma;
    float mix;
};
static_assert(sizeof(LineSDFUBO) % 16 == 0);

struct alignas(16) LineSDFPropertiesUBO {
    Color color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    float pad1, pad2;
};
static_assert(sizeof(LineSDFPropertiesUBO) % 16 == 0);

using LineBasicUBO = LineUBO;

struct alignas(16) LineBasicPropertiesUBO {
    Color color;
    float opacity;
    float width;
    float pad1, pad2;
};
static_assert(sizeof(LineBasicPropertiesUBO) % 16 == 0);

/// Property interpolation UBOs
struct alignas(16) LineInterpolationUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1, pad2;
};
static_assert(sizeof(LineInterpolationUBO) % 16 == 0);

struct alignas(16) LineGradientInterpolationUBO {
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1, pad2, pad3;
};
static_assert(sizeof(LineGradientInterpolationUBO) % 16 == 0);

struct alignas(16) LinePatternInterpolationUBO {
    float blur_t;
    float opacity_t;
    float offset_t;
    float gapwidth_t;
    float width_t;
    float pattern_from_t;
    float pattern_to_t;
    float pad1;
};
static_assert(sizeof(LinePatternInterpolationUBO) % 16 == 0);

struct alignas(16) LineSDFInterpolationUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float floorwidth_t;
    float pad1;
};
static_assert(sizeof(LineSDFInterpolationUBO) % 16 == 0);

/// Evaluated properties that depend on the tile
struct alignas(16) LinePatternTilePropertiesUBO {
    std::array<float, 4> pattern_from;
    std::array<float, 4> pattern_to;
};
static_assert(sizeof(LinePatternTilePropertiesUBO) % 16 == 0);

struct alignas(16) LinePermutationUBO {
    /*  0 */ Attribute color;
    /*  8 */ Attribute blur;
    /* 16 */ Attribute opacity;
    /* 24 */ Attribute gapwidth;
    /* 32 */ Attribute offset;
    /* 40 */ Attribute width;
    /* 48 */ Attribute floorwidth;
    /* 56 */ Attribute pattern_from;
    /* 64 */ Attribute pattern_to;
    /* 72 */ bool overdrawInspector;
    /* 73 */ uint8_t pad1, pad2, pad3;
    /* 76 */ float pad4;
    /* 80 */
};
static_assert(sizeof(LinePermutationUBO) == 5 * 16);

} // namespace shaders
} // namespace mbgl
