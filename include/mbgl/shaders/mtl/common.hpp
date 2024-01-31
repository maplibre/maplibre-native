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

struct alignas(16) LineDynamicUBO {
    float2 units_to_pixels;
    float pad1, pad2;
};

struct alignas(16) LineUBO {
    float4x4 matrix;
    float ratio;
    float pad1, pad2, pad3;
};

struct alignas(16) LineBasicUBO {
    float4x4 matrix;
    float2 units_to_pixels;
    float ratio;
    float pad;
};

struct alignas(16) LineGradientUBO {
    float4x4 matrix;
    float ratio;
    float pad1, pad2, pad3;
};

struct alignas(16) LinePropertiesUBO {
    float4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float pad1, pad2, pad3;
};

struct alignas(16) LineBasicPropertiesUBO {
    float4 color;
    float opacity;
    float width;
    float pad1, pad2;
};

struct alignas(16) LineGradientPropertiesUBO {
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float pad1, pad2, pad3;
};

struct alignas(16) LineInterpolationUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1, pad2;
};

struct alignas(16) LineGradientInterpolationUBO {
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1, pad2, pad3;
};

struct alignas(16) SymbolDrawableTilePropsUBO {
    /*bool*/ int is_text;
    /*bool*/ int is_halo;
    /*bool*/ int pitch_with_map;
    /*bool*/ int is_size_zoom_constant;
    /*bool*/ int is_size_feature_constant;
    float size_t;
    float size;
    float padding;
};
static_assert(sizeof(SymbolDrawableTilePropsUBO) == 2 * 16, "unexpected padding");

struct alignas(16) SymbolDrawableInterpolateUBO {
    float fill_color_t;
    float halo_color_t;
    float opacity_t;
    float halo_width_t;
    float halo_blur_t;
    float pad1, pad2, pad3;
};
static_assert(sizeof(SymbolDrawableInterpolateUBO) == 32, "unexpected padding");

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

struct alignas(16) SymbolDynamicUBO {
    float fade_change;
    float camera_to_center_distance;
    float aspect_ratio;
    float pad;
};
static_assert(sizeof(SymbolDynamicUBO) == 16, "unexpected padding");

struct alignas(16) SymbolDrawablePaintUBO {
    float4 fill_color;
    float4 halo_color;
    float opacity;
    float halo_width;
    float halo_blur;
    float padding;
};
static_assert(sizeof(SymbolDrawablePaintUBO) == 3 * 16, "unexpected padding");

// unpack pattern position
inline float2 get_pattern_pos(const float2 pixel_coord_upper, const float2 pixel_coord_lower,
                     const float2 pattern_size, const float tile_units_to_pixels, const float2 pos) {
    const float2 offset = glMod(glMod(glMod(pixel_coord_upper, pattern_size) * 256.0, pattern_size) * 256.0 + pixel_coord_lower, pattern_size);
    return (tile_units_to_pixels * pos + offset) / pattern_size;
}

)";

}
} // namespace mbgl
