#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::Prelude, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "Prelude";

    static constexpr auto vertex = R"(

#define M_PI 3.1415926535897932384626433832795

// The maximum allowed miter limit is 2.0 at the moment. the extrude normal is stored
// in a byte (-128..127). We scale regular normals up to length 63, but there are also
// "special" normals that have a bigger length (of up to 126 in this case).
#define LINE_NORMAL_SCALE (1.0 / (127 / 2))

// The attribute conveying progress along a line is scaled to [0, 2^15).
#define MAX_LINE_DISTANCE 32767.0

// Unpack a pair of values that have been packed into a single float.
// The packed values are assumed to be 8-bit unsigned integers, and are
// packed like so:
// packedValue = floor(input[0]) * 256 + input[1],
vec2 unpack_float(const float packedValue) {
    int packedIntValue = int(packedValue);
    int v0 = packedIntValue / 256;
    return vec2(v0, packedIntValue - v0 * 256);
}

vec2 unpack_opacity(const float packedOpacity) {
    int intOpacity = int(packedOpacity) / 2;
    return vec2(float(intOpacity) / 127.0, mod(packedOpacity, 2.0));
}

// To minimize the number of attributes needed, we encode a 4-component
// color into a pair of floats (i.e. a vec2) as follows:
// [ floor(color.r * 255) * 256 + color.g * 255,
//   floor(color.b * 255) * 256 + color.g * 255 ]
vec4 decode_color(const vec2 encodedColor) {
    return vec4(
        unpack_float(encodedColor[0]) / 255.0,
        unpack_float(encodedColor[1]) / 255.0
    );
}

// Unpack a pair of paint values and interpolate between them.
float unpack_mix_float(const vec2 packedValue, const float t) {
    return mix(packedValue[0], packedValue[1], t);
}

// Unpack a pair of paint values and interpolate between them.
vec4 unpack_mix_color(const vec4 packedColors, const float t) {
    vec4 minColor = decode_color(vec2(packedColors[0], packedColors[1]));
    vec4 maxColor = decode_color(vec2(packedColors[2], packedColors[3]));
    return mix(minColor, maxColor, t);
}

// unpack pattern position
vec2 get_pattern_pos(const vec2 pixel_coord_upper, const vec2 pixel_coord_lower,
                        const vec2 pattern_size, const float tile_units_to_pixels, const vec2 pos) {
    const vec2 offset = mod(mod(mod(pixel_coord_upper, pattern_size) * 256.0, pattern_size) * 256.0 + pixel_coord_lower, pattern_size);
    return (tile_units_to_pixels * pos + offset) / pattern_size;
}

vec2 unpack_int(int value) {
    const int low  = (value << 16) >> 16;
    const int high = value >> 16;

    return vec2(low, high);
}

vec2 unpack_uint(uint value) {
    const uint low  = value & 0xFFFF;
    const uint high = (value >> 16) & 0xFFFF;

    return vec2(low, high);
}

#define GLOBAL_SET_INDEX                    0
#define LAYER_SET_INDEX                     1
#define DRAWABLE_UBO_SET_INDEX              2
#define DRAWABLE_IMAGE_SET_INDEX            3

#define layerSSBOStartId                    0
#define layerUBOStartId                     3
#define drawableSSBOStartId                 0
#define drawableUBOStartId                  1

#define idDrawableReservedVertexOnlyUBO     layerSSBOStartId
#define idDrawableReservedFragmentOnlyUBO   idDrawableReservedVertexOnlyUBO + 1
#define drawableReservedUBOCount            idDrawableReservedFragmentOnlyUBO + 1

layout(set = GLOBAL_SET_INDEX, binding = 0) uniform GlobalPaintParamsUBO {
    vec2 pattern_atlas_texsize;
    vec2 units_to_pixels;
    vec2 world_size;
    float camera_to_center_distance;
    float symbol_fade_change;
    float aspect_ratio;
    float pixel_ratio;
    float map_zoom;
    float pad1;
} paintParams;

#ifdef USE_SURFACE_TRANSFORM
layout(set = GLOBAL_SET_INDEX, binding = 1) uniform GlobalPlatformParamsUBO {
    vec4 surfaceRotation;
} platformParams;
#endif

void applySurfaceTransform() {
#ifdef USE_SURFACE_TRANSFORM
    const mat2 rotation = {
        platformParams.surfaceRotation.xy,
        platformParams.surfaceRotation.zw
    };
    gl_Position.xy = rotation * gl_Position.xy;
#endif

    gl_Position.y *= -1.0;
}

)";

    static constexpr auto fragment = R"(

#define M_PI 3.1415926535897932384626433832795
#define SDF_PX 8.0

#define GLOBAL_SET_INDEX                    0
#define LAYER_SET_INDEX                     1
#define DRAWABLE_UBO_SET_INDEX              2
#define DRAWABLE_IMAGE_SET_INDEX            3

#define layerSSBOStartId                    0
#define layerUBOStartId                     3
#define drawableSSBOStartId                 0
#define drawableUBOStartId                  1

#define idDrawableReservedVertexOnlyUBO     layerSSBOStartId
#define idDrawableReservedFragmentOnlyUBO   idDrawableReservedVertexOnlyUBO + 1
#define drawableReservedUBOCount            idDrawableReservedFragmentOnlyUBO + 1

layout(set = GLOBAL_SET_INDEX, binding = 0) uniform GlobalPaintParamsUBO {
    vec2 pattern_atlas_texsize;
    vec2 units_to_pixels;
    vec2 world_size;
    float camera_to_center_distance;
    float symbol_fade_change;
    float aspect_ratio;
    float pixel_ratio;
    float map_zoom;
    float pad1;
} paintParams;

)";
};

} // namespace shaders
} // namespace mbgl
