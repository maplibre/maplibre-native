#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto backgroundShaderPrelude = R"(

#define idBackgroundDrawableUBO     idDrawableReservedVertexOnlyUBO
#define idBackgroundPropsUBO        layerUBOStartId

)";

template <>
struct ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "BackgroundShader";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = backgroundShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct BackgroundDrawableUBO {
    mat4 matrix;
    vec4 pad1;
    vec4 pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idBackgroundDrawableUBO) readonly buffer BackgroundDrawableUBOVector {
    BackgroundDrawableUBO drawable_ubo[];
} drawableVector;

void main() {
    const BackgroundDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idBackgroundPropsUBO) uniform BackgroundPropsUBO {
    vec4 color;
    float opacity;
    float pad1;
    float pad2;
    float pad3;
} props;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    out_color = props.color * props.opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "BackgroundPatternShader";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = backgroundShaderPrelude;
    static constexpr auto vertex = R"(
layout(location = 0) in ivec2 in_position;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct BackgroundPatternDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float tile_units_to_pixels;
    float pad1;
    float pad2;
    float pad3;
};

layout(std140, set = LAYER_SET_INDEX, binding = idBackgroundDrawableUBO) readonly buffer BackgroundPatternDrawableUBOVector {
    BackgroundPatternDrawableUBO drawable_ubo[];
} drawableVector;

layout(set = LAYER_SET_INDEX, binding = idBackgroundPropsUBO) uniform BackgroundPatternPropsUBO {
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
} props;

layout(location = 0) out vec2 frag_pos_a;
layout(location = 1) out vec2 frag_pos_b;

void main() {
    const BackgroundPatternDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

    frag_pos_a = get_pattern_pos(drawable.pixel_coord_upper,
                                 drawable.pixel_coord_lower,
                                 props.scale_a * props.pattern_size_a,
                                 drawable.tile_units_to_pixels,
                                 in_position);
    frag_pos_b = get_pattern_pos(drawable.pixel_coord_upper,
                                 drawable.pixel_coord_lower,
                                 props.scale_b * props.pattern_size_b,
                                 drawable.tile_units_to_pixels,
                                 in_position);

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) in vec2 frag_pos_a;
layout(location = 1) in vec2 frag_pos_b;

layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idBackgroundPropsUBO) uniform BackgroundPatternLayerUBO {
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
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const vec2 texsize = paintParams.pattern_atlas_texsize;

    const vec2 imagecoord = mod(frag_pos_a, 1.0);
    const vec2 pos = mix(props.pattern_tl_a / texsize, props.pattern_br_a / texsize, imagecoord);
    const vec4 color1 = texture(image_sampler, pos);

    const vec2 imagecoord_b = mod(frag_pos_b, 1.0);
    const vec2 pos2 = mix(props.pattern_tl_b / texsize, props.pattern_br_b / texsize, imagecoord_b);
    const vec4 color2 = texture(image_sampler, pos2);

    out_color = mix(color1, color2, props.mix) * props.opacity;
}
)";
};

} // namespace shaders
} // namespace mbgl
