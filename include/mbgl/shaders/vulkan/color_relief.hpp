#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto colorReliefShaderPrelude = R"(

#define idColorReliefDrawableUBO        idDrawableReservedVertexOnlyUBO
#define idColorReliefTilePropsUBO       idDrawableReservedFragmentOnlyUBO
#define idColorReliefEvaluatedPropsUBO  layerUBOStartId

)";

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

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idColorReliefDrawableUBO) uniform ColorReliefDrawableUBO {
    mat4 matrix;
} drawable;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idColorReliefTilePropsUBO) uniform ColorReliefTilePropsUBO {
    vec4 unpack;
    vec2 dimension;
    int color_ramp_size;
    float pad_tile0;
} tileProps;

layout(location = 0) out vec2 frag_position;

void main() {

    gl_Position = drawable.matrix * vec4(in_position, 0, 1);
    applySurfaceTransform();

    highp vec2 epsilon = 1.0 / tileProps.dimension;
    float scale = (tileProps.dimension.x - 2.0) / tileProps.dimension.x;
    frag_position = (vec2(in_position) / 8192.0) * scale + epsilon;

    // Handle poles (use in_position to match GLSL a_pos)
    if (float(in_position.y) < -32767.5) frag_position.y = 0.0;
    if (float(in_position.y) > 32766.5) frag_position.y = 1.0;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_position;
layout(location = 0) out vec4 out_color;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idColorReliefTilePropsUBO) uniform ColorReliefTilePropsUBO {
    vec4 unpack;
    vec2 dimension;
    int color_ramp_size;
    float pad_tile0;
} tileProps;

layout(set = LAYER_SET_INDEX, binding = idColorReliefEvaluatedPropsUBO) uniform ColorReliefEvaluatedPropsUBO {
    float opacity;
    float pad_eval0;
    float pad_eval1;
    float pad_eval2;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image_sampler;
layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 1) uniform sampler2D elevation_stops_sampler;
layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 2) uniform sampler2D color_stops_sampler;

float getElevation(vec2 coord) {
    // Convert encoded elevation value to meters
    vec4 data = texture(image_sampler, coord) * 255.0;
    data.a = -1.0;
    return dot(data, tileProps.unpack);
}

float getElevationStop(int stop) {
    // Elevation stops are plain float values, not terrain-RGB encoded
    float x = (float(stop) + 0.5) / float(tileProps.color_ramp_size);
    return texture(elevation_stops_sampler, vec2(x, 0.0)).r;
}

vec4 getColorStop(int stop) {
    float x = (float(stop) + 0.5) / float(tileProps.color_ramp_size);
    return texture(color_stops_sampler, vec2(x, 0.0));
}

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    float el = getElevation(frag_position);

    // Binary search to find surrounding elevation stops (l and r indices)
    int r = tileProps.color_ramp_size - 1;
    int l = 0;

    // Perform binary search
    while (r - l > 1) {
        int m = (r + l) / 2;
        float el_m = getElevationStop(m);
        
        if (el < el_m) {
            r = m;
        } else {
            l = m;
        }
    }

    // Get elevation values and colors at the stops
    float el_l = getElevationStop(l);
    float el_r = getElevationStop(r);

    vec4 color_l = getColorStop(l);
    vec4 color_r = getColorStop(r);

    // Interpolate color based on elevation
    // Guard against division by zero when el_r == el_l
    float denom = el_r - el_l;
    float t = (abs(denom) < 0.0001) ? 0.0 : clamp((el - el_l) / denom, 0.0, 1.0);

    out_color = props.opacity * mix(color_l, color_r, t);
}
)";
};

} // namespace shaders
} // namespace mbgl
