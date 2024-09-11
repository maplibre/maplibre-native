#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "BackgroundShader";

    static const std::array<UniformBlockInfo, 2> uniforms;
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

layout(set = 0, binding = 1) uniform BackgroundDrawableUBO {
    mat4 matrix;
} drawable;

void main() {
    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    gl_Position.y *= -1.0;
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 2) uniform BackgroundLayerUBO {
    vec4 color;
    float opacity;
    float pad1, pad2, pad3;
} layer;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    out_color = layer.color * layer.opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "BackgroundPatternShader";

    static const std::array<UniformBlockInfo, 3> uniforms;
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

layout(set = 0, binding = 1) uniform BackgroundPatternDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float tile_units_to_pixels;
    float pad1, pad2, pad3;
} drawable;

layout(set = 0, binding = 2) uniform BackgroundPatternLayerUBO {
    vec2 pattern_tl_a;
    vec2 pattern_br_a;
    vec2 pattern_tl_b;
    vec2 pattern_br_b;
    vec2 pattern_size_a;
    vec2 pattern_size_b;
    float scale_a;
    float scale_b;
    float mix;
    float opacity;
} layer;

layout(location = 0) out vec2 frag_pos_a;
layout(location = 1) out vec2 frag_pos_b;

void main() {

    frag_pos_a = get_pattern_pos(drawable.pixel_coord_upper,
                                 drawable.pixel_coord_lower,
                                 layer.scale_a * layer.pattern_size_a,
                                 drawable.tile_units_to_pixels,
                                 in_position);
    frag_pos_b = get_pattern_pos(drawable.pixel_coord_upper,
                                 drawable.pixel_coord_lower,
                                 layer.scale_b * layer.pattern_size_b,
                                 drawable.tile_units_to_pixels,
                                 in_position);

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    gl_Position.y *= -1.0;
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) in vec2 frag_pos_a;
layout(location = 1) in vec2 frag_pos_b;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 2) uniform BackgroundPatternLayerUBO {
    vec2 pattern_tl_a;
    vec2 pattern_br_a;
    vec2 pattern_tl_b;
    vec2 pattern_br_b;
    vec2 pattern_size_a;
    vec2 pattern_size_b;
    float scale_a;
    float scale_b;
    float mix;
    float opacity;
} layer;

layout(set = 1, binding = 0) uniform sampler2D image_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const vec2 texsize = global.pattern_atlas_texsize;

    const vec2 imagecoord = mod(frag_pos_a, 1.0);
    const vec2 pos = mix(layer.pattern_tl_a / texsize, layer.pattern_br_a / texsize, imagecoord);
    const vec4 color1 = texture(image_sampler, pos);

    const vec2 imagecoord_b = mod(frag_pos_b, 1.0);
    const vec2 pos2 = mix(layer.pattern_tl_b / texsize, layer.pattern_br_b / texsize, imagecoord_b);
    const vec4 color2 = texture(image_sampler, pos2);

    out_color = mix(color1, color2, layer.mix) * layer.opacity;
}
)";
};

} // namespace shaders
} // namespace mbgl
