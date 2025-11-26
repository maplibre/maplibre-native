#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

constexpr auto colorReliefShaderPrelude = R"(\n
#define idColorReliefDrawableUBO        idDrawableReservedVertexOnlyUBO
#define idColorReliefTilePropsUBO       idDrawableReservedFragmentOnlyUBO
#define idColorReliefEvaluatedPropsUBO  layerUBOStartId
\n)";

template <>
struct ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "ColorReliefShader";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 3> textures;

    static constexpr auto prelude = colorReliefShaderPrelude;

    static constexpr auto vertex = R"(
layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec2 in_texture_position;

// Push Constant for UBO index (Vulkan pattern)
layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

// Matches ColorReliefDrawableUBO
struct ColorReliefDrawableUBO {
    mat4 matrix;
};

// Drawable UBO Vector (Readonly Buffer)
layout(std140, set = DRAWABLE_UBO_SET_INDEX, binding = idColorReliefDrawableUBO) readonly buffer ColorReliefDrawableUBOVector {
    ColorReliefDrawableUBO drawable_ubo[];
} drawableVector;

// Matches ColorReliefTilePropsUBO
struct ColorReliefTilePropsUBO {
    vec4 unpack;
    vec2 dimension;
    int color_ramp_size;
    float pad_tile0;
};

// Tile Props UBO Vector (Readonly Buffer)
layout(std140, set = DRAWABLE_UBO_SET_INDEX, binding = idColorReliefTilePropsUBO) readonly buffer ColorReliefTilePropsUBOVector {
    ColorReliefTilePropsUBO tileProps_ubo[];
} tilePropsVector;

layout(location = 0) out vec2 frag_position;

void main() {
    const ColorReliefDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];
    const ColorReliefTilePropsUBO tileProps = tilePropsVector.tileProps_ubo[constant.ubo_index];

    gl_Position = drawable.matrix * vec4(in_position, 0, 1);

    highp vec2 a_pos = vec2(in_texture_position);

    highp vec2 epsilon = 1.0 / tileProps.dimension;
    float scale = (tileProps.dimension.x - 2.0) / tileProps.dimension.x;
    frag_position = (a_pos / 8192.0) * scale + epsilon;

    // Handle poles
    if (a_pos.y < -32767.5) frag_position.y = 0.0;
    if (a_pos.y > 32766.5) frag_position.y = 1.0;
}
)"_glsl;

    static constexpr auto fragment = R"(
#if defined(GL_ES) && defined(VULKAN)
precision highp float;
#endif

// Push Constant for UBO index (Vulkan pattern)
layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

// Matches ColorReliefTilePropsUBO
struct ColorReliefTilePropsUBO {
    vec4 unpack;
    vec2 dimension;
    int color_ramp_size;
    float pad_tile0;
};

// Tile Props UBO Vector (Readonly Buffer)
layout(std140, set = DRAWABLE_UBO_SET_INDEX, binding = idColorReliefTilePropsUBO) readonly buffer ColorReliefTilePropsUBOVector {
    ColorReliefTilePropsUBO tileProps_ubo[];
} tilePropsVector;

// Matches ColorReliefEvaluatedPropsUBO
struct ColorReliefEvaluiefEvaluatedPropsUBO {
    float opacity;
    float pad_eval0;
    float pad_eval1;
    float pad_eval2;
};

// Evaluated Props UBO Vector (Readonly Buffer)
layout(std140, set = LAYER_SET_INDEX, binding = idColorReliefEvaluatedPropsUBO) readonly buffer ColorReliefEvaluatedPropsUBOVector {
    ColorReliefEvaluatedPropsUBO props_ubo[];
} propsVector;


layout(set = BINDINGS_SET_INDEX, binding = 0) uniform sampler2D u_image;
layout(set = BINDINGS_SET_INDEX, binding = 1) uniform sampler2D u_elevation_stops;
layout(set = BINDINGS_SET_INDEX, binding = 2) uniform sampler2D u_color_stops;

layout(location = 0) in vec2 frag_position;
layout(location = 0) out vec4 out_color;

float getElevation(vec2 coord, const vec4 unpack) {
    // Convert encoded elevation value to meters
    vec4 data = texture(u_image, coord) * 255.0;
    data.a = -1.0;
    return dot(data, unpack);
}

float getElevationStop(int stop, const int color_ramp_size) {
    // Elevation stops are plain float values, not terrain-RGB encoded
    float x = (float(stop) + 0.5) / float(color_ramp_size);
    return texture(u_elevation_stops, vec2(x, 0.0)).r;
}

vec4 getColorStop(int stop, const int color_ramp_size) {
    float x = (float(stop) + 0.5) / float(color_ramp_size);
    return texture(u_color_stops, vec2(x, 0.0));
}

void main() {
    const ColorReliefTilePropsUBO tileProps = tilePropsVector.tileProps_ubo[constant.ubo_index];
    const ColorReliefEvaluatedPropsUBO props = propsVector.props_ubo[constant.ubo_index];
    
    float el = getElevation(frag_position, tileProps.unpack);

    // Binary search for color stops
    int r = (tileProps.color_ramp_size - 1);
    int l = 0;

    while (r - l > 1) {
        int m = (r + l) / 2;
        float el_m = getElevationStop(m, tileProps.color_ramp_size);
        if (el < el_m) {
            r = m;
        } else {
            l = m;
        }
    }

    // Get elevation values for interpolation
    float el_l = getElevationStop(l, tileProps.color_ramp_size);
    float el_r = getElevationStop(r, tileProps.color_ramp_size);

    // Get color values for interpolation
    vec4 color_l = getColorStop(l, tileProps.color_ramp_size);
    vec4 color_r = getColorStop(r, tileProps.color_ramp_size);

    // Interpolate
    float t = (el - el_l) / (el_r - el_l);
    vec4 color = mix(color_l, color_r, t);

    out_color = color * props.opacity;
}
)"_glsl;
};

} // namespace shaders
} // namespace mbgl
