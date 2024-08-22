#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "RasterShader";

    static const std::array<UniformBlockInfo, 2> uniforms;
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec2 in_texture_position;

layout(set = 0, binding = 1) uniform RasterDrawableUBO {
    mat4 matrix;
} drawable;

layout(set = 0, binding = 2) uniform RasterEvaluatedPropsUBO {
    vec4 spin_weights;
    vec2 tl_parent;
    float scale_parent;
    float buffer_scale;
    float fade_t;
    float opacity;
    float brightness_low;
    float brightness_high;
    float saturation_factor;
    float contrast_factor;
    float pad1, pad2;
} props;

layout(location = 0) out vec2 frag_position0;
layout(location = 1) out vec2 frag_position1;

void main() {

    gl_Position = drawable.matrix * vec4(in_position, 0, 1);
    gl_Position.y *= -1.0;

    // We are using Int16 for texture position coordinates to give us enough precision for
    // fractional coordinates. We use 8192 to scale the texture coordinates in the buffer
    // as an arbitrarily high number to preserve adequate precision when rendering.
    // This is also the same value as the EXTENT we are using for our tile buffer pos coordinates,
    // so math for modifying either is consistent.
    frag_position0 = (((in_texture_position / 8192.0) - 0.5) / props.buffer_scale ) + 0.5;
    frag_position1 = (frag_position0 * props.scale_parent) + props.tl_parent;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_position0;
layout(location = 1) in vec2 frag_position1;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 2) uniform RasterEvaluatedPropsUBO {
    vec4 spin_weights;
    vec2 tl_parent;
    float scale_parent;
    float buffer_scale;
    float fade_t;
    float opacity;
    float brightness_low;
    float brightness_high;
    float saturation_factor;
    float contrast_factor;
    float pad1, pad2;
} props;

layout(set = 1, binding = 0) uniform sampler2D image0_sampler;
layout(set = 1, binding = 1) uniform sampler2D image1_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    // read and cross-fade colors from the main and parent tiles
    vec4 color0 = texture(image0_sampler, frag_position0);
    vec4 color1 = texture(image1_sampler, frag_position1);
    if (color0.a > 0.0) {
        color0.rgb = color0.rgb / color0.a;
    }
    if (color1.a > 0.0) {
        color1.rgb = color1.rgb / color1.a;
    }
    vec4 color = mix(color0, color1, props.fade_t);
    color.a *= props.opacity;
    vec3 rgb = color.rgb;

    // spin
    rgb = vec3(
        dot(rgb, props.spin_weights.xyz),
        dot(rgb, props.spin_weights.zxy),
        dot(rgb, props.spin_weights.yzx));

    // saturation
    float average = (color.r + color.g + color.b) / 3.0;
    rgb += (average - rgb) * props.saturation_factor;

    // contrast
    rgb = (rgb - 0.5) * props.contrast_factor + 0.5;

    // brightness
    vec3 high_vec = vec3(props.brightness_low, props.brightness_low, props.brightness_low);
    vec3 low_vec = vec3(props.brightness_high, props.brightness_high, props.brightness_high);

    out_color = vec4(vec3(mix(high_vec, low_vec, rgb) * color.a), color.a);
}
)";
};

} // namespace shaders
} // namespace mbgl
