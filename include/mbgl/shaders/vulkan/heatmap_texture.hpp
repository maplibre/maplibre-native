#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto heatmapTextureShaderPrelude = R"(#define idHeatmapTexturePropsUBO    layerUBOStartId)";

template <>
struct ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "HeatmapTextureShader";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto prelude = heatmapTextureShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

layout(set = LAYER_SET_INDEX, binding = idHeatmapTexturePropsUBO) uniform HeatmapTexturePropsUBO {
    mat4 matrix;
    float opacity;
    float pad1;
    float pad2;
    float pad3;
} props;

layout(location = 0) out vec2 frag_position;

void main() {

    gl_Position = props.matrix * vec4(in_position * paintParams.world_size, 0, 1);
    applySurfaceTransform();

    frag_position = in_position;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_position;
layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idHeatmapTexturePropsUBO) uniform HeatmapTexturePropsUBO {
    mat4 matrix;
    float opacity;
    float pad1;
    float pad2;
    float pad3;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image_sampler;
layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 1) uniform sampler2D color_ramp_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const float t = texture(image_sampler, frag_position).r;
    const vec4 color = texture(color_ramp_sampler, vec2(t, 0.5));
    out_color = color * props.opacity;
}
)";
};

} // namespace shaders
} // namespace mbgl
