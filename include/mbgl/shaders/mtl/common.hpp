#pragma once

namespace mbgl {
namespace shaders {

constexpr auto prelude = R"(
#include <metal_stdlib>
using namespace metal;

// The maximum allowed miter limit is 2.0 at the moment. the extrude normal is stored
// in a byte (-128..127). We scale regular normals up to length 63, but there are also
// "special" normals that have a bigger length (of up to 126 in this case).
#define LINE_NORMAL_SCALE (1.0 / (127 / 2))

// The attribute conveying progress along a line is scaled to [0, 2^15).
#define MAX_LINE_DISTANCE 32767.0

#define SDF_PX 8.0

// OpenGL `mod` is `x-y*floor(x/y)` where `floor` rounds down.
// Metal `fmod` is `x-y*trunc(x/y)` where `trunc` rounds toward zero.
// This function provides GL-compatible modulus for porting GLSL shaders.
template <typename T1, typename T2>
inline auto glMod(T1 x, T2 y) { return x - y * metal::floor(x/y); }

float radians(float degrees) {
    return M_PI_F * degrees / 180.0;
}

// Unpack a pair of values that have been packed into a single float.
// The packed values are assumed to be 8-bit unsigned integers, and are
// packed like so: packedValue = floor(input[0]) * 256 + input[1],
float2 unpack_float(const float packedValue) {
    const int packedIntValue = int(packedValue);
    const int v0 = packedIntValue / 256;
    return float2(v0, packedIntValue - v0 * 256);
}
float2 unpack_opacity(const float packedOpacity) {
    return float2(float(int(packedOpacity) / 2) / 127.0, glMod(packedOpacity, 2.0));
}
// To minimize the number of attributes needed, we encode a 4-component
// color into a pair of floats (i.e. a vec2) as follows:
// [ floor(color.r * 255) * 256 + color.g * 255, floor(color.b * 255) * 256 + color.g * 255 ]
float4 decode_color(const float2 encoded) {
    return float4(unpack_float(encoded[0]) / 255, unpack_float(encoded[1]) / 255);
}
// Unpack a pair of paint values and interpolate between them.
float unpack_mix_float(const float2 packedValue, const float t) {
    return mix(packedValue[0], packedValue[1], t);
}
// Unpack a pair of paint values and interpolate between them.
float4 unpack_mix_color(const float4 packedColors, const float t) {
    return mix(decode_color(float2(packedColors[0], packedColors[1])),
               decode_color(float2(packedColors[2], packedColors[3])), t);
}

struct alignas(16) FillEvaluatedPropsUBO {
    float4 color;
    float4 outline_color;
    float opacity;
    float fade;
    float width;
    float pad1;
};

struct alignas(16) FillExtrusionDrawableUBO {
    /*   0 */ float4x4 matrix;
    /*  64 */ float4 scale;
    /*  80 */ float2 texsize;
    /*  88 */ float2 pixel_coord_upper;
    /*  96 */ float2 pixel_coord_lower;
    /* 104 */ float height_factor;
    /* 108 */ float pad;
    /* 112 */
};
static_assert(sizeof(FillExtrusionDrawableUBO) == 7 * 16, "unexpected padding");

struct alignas(16) FillExtrusionPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float4 light_color_pad;
    /* 32 */ float4 light_position_base;
    /* 48 */ float height;
    /* 52 */ float light_intensity;
    /* 56 */ float vertical_gradient;
    /* 60 */ float opacity;
    /* 64 */ float fade;
    /* 68 */ float pad2, pad3, pad4;
    /* 80 */
};
static_assert(sizeof(FillExtrusionPropsUBO) == 5 * 16, "unexpected padding");

struct alignas(16) FillExtrusionTilePropsUBO {
    /*  0 */ float4 pattern_from;
    /* 16 */ float4 pattern_to;
    /* 32 */
};
static_assert(sizeof(FillExtrusionTilePropsUBO) == 2 * 16, "unexpected padding");

struct alignas(16) FillExtrusionInterpolateUBO {
    /*  0 */ float base_t;
    /*  4 */ float height_t;
    /*  8 */ float color_t;
    /* 12 */ float pattern_from_t;
    /* 16 */ float pattern_to_t;
    /* 20 */ float pad1, pad2, pad3;
    /* 32 */
};
static_assert(sizeof(FillExtrusionInterpolateUBO) == 2 * 16, "unexpected padding");

struct alignas(16) LineDynamicUBO {
    float2 units_to_pixels;
    float pad1, pad2;
};

struct alignas(16) LineEvaluatedPropsUBO {
    float4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    float pad1, pad2;
};

struct alignas(16) SymbolDynamicUBO {
    float fade_change;
    float camera_to_center_distance;
    float aspect_ratio;
    float pad;
};
static_assert(sizeof(SymbolDynamicUBO) == 16, "unexpected padding");

struct alignas(16) SymbolDrawableUBO {
    float4x4 matrix;
    float4x4 label_plane_matrix;
    float4x4 coord_matrix;

    float2 texsize;
    float2 texsize_icon;

    float gamma_scale;
    /*bool*/ int rotate_symbol;
    float2 pad;
};
static_assert(sizeof(SymbolDrawableUBO) == 14 * 16, "unexpected padding");

struct alignas(16) SymbolTilePropsUBO {
    /*bool*/ int is_text;
    /*bool*/ int is_halo;
    /*bool*/ int pitch_with_map;
    /*bool*/ int is_size_zoom_constant;
    /*bool*/ int is_size_feature_constant;
    float size_t;
    float size;
    float padding;
};
static_assert(sizeof(SymbolTilePropsUBO) == 2 * 16, "unexpected padding");

struct alignas(16) SymbolInterpolateUBO {
    float fill_color_t;
    float halo_color_t;
    float opacity_t;
    float halo_width_t;
    float halo_blur_t;
    float pad1, pad2, pad3;
};
static_assert(sizeof(SymbolInterpolateUBO) == 32, "unexpected padding");

struct alignas(16) SymbolEvaluatedPropsUBO {
    float4 text_fill_color;
    float4 text_halo_color;
    float text_opacity;
    float text_halo_width;
    float text_halo_blur;
    float pad1;
    float4 icon_fill_color;
    float4 icon_halo_color;
    float icon_opacity;
    float icon_halo_width;
    float icon_halo_blur;
    float pad2;
};
static_assert(sizeof(SymbolEvaluatedPropsUBO) == 6 * 16, "unexpected padding");

// unpack pattern position
inline float2 get_pattern_pos(const float2 pixel_coord_upper, const float2 pixel_coord_lower,
                     const float2 pattern_size, const float tile_units_to_pixels, const float2 pos) {
    const float2 offset = glMod(glMod(glMod(pixel_coord_upper, pattern_size) * 256.0, pattern_size) * 256.0 + pixel_coord_lower, pattern_size);
    return (tile_units_to_pixels * pos + offset) / pattern_size;
}

)";

}
} // namespace mbgl
