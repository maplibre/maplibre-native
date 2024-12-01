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

#define GLOBAL_SET_INDEX            0
#define LAYER_SET_INDEX             1
#define DRAWABLE_UBO_SET_INDEX      2
#define DRAWABLE_IMAGE_SET_INDEX    3

layout(set = GLOBAL_SET_INDEX, binding = 0) uniform GlobalPaintParamsUBO {
    vec2 pattern_atlas_texsize;
    vec2 units_to_pixels;
    vec2 world_size;
    float camera_to_center_distance;
    float symbol_fade_change;
    float aspect_ratio;
    float pixel_ratio;
    float zoom;
    float pad1;
} global;

#ifdef USE_SURFACE_TRANSFORM
layout(set = GLOBAL_SET_INDEX, binding = 1) uniform PlatformParamsUBO {
    mat2 rotation;
} platform;
#endif

void applySurfaceTransform() {
#ifdef USE_SURFACE_TRANSFORM
    gl_Position.xy = platform.rotation * gl_Position.xy;
#endif

    gl_Position.y *= -1.0;
}

)";

    static constexpr auto fragment = R"(

#define M_PI 3.1415926535897932384626433832795
#define SDF_PX 8.0

#define GLOBAL_SET_INDEX            0
#define LAYER_SET_INDEX             1
#define DRAWABLE_UBO_SET_INDEX      2
#define DRAWABLE_IMAGE_SET_INDEX    3

layout(set = GLOBAL_SET_INDEX, binding = 0) uniform GlobalPaintParamsUBO {
    vec2 pattern_atlas_texsize;
    vec2 units_to_pixels;
    vec2 world_size;
    float camera_to_center_distance;
    float symbol_fade_change;
    float aspect_ratio;
    float pixel_ratio;
    float zoom;
    float pad1;
} global;

)";
};

template <>
struct ShaderSource<BuiltIn::CommonShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "CommonShader";

    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(
layout(location = 0) in vec2 in_position;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = 0) uniform CommonUBO {
    mat4 matrix;
    vec4 color;
} ubo;

void main() {
    gl_Position = ubo.matrix * vec4(in_position, 0, 1);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) out vec4 out_color;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = 0) uniform CommonUBO {
    mat4 matrix;
    vec4 color;
} ubo;

void main() {
    out_color = ubo.color;
}
)";
};

template <>
struct ShaderSource<BuiltIn::CommonTexturedShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "CommonTexturedShader";

    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = 0) uniform CommonUBO {
    mat4 matrix;
    vec4 color;
} ubo;

layout(location = 0) out vec2 frag_uv;

void main() {
    gl_Position = ubo.matrix * vec4(in_position, 0, 1);
    applySurfaceTransform();

    frag_uv = in_texcoord;
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) in vec2 frag_uv;
layout(location = 0) out vec4 out_color;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image_sampler;

void main() {
    out_color = texture(image_sampler, frag_uv);
}
)";
};

} // namespace shaders
} // namespace mbgl
