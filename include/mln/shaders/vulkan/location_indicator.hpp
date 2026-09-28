#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/vulkan/shader_program.hpp>

namespace mln {
namespace shaders {

constexpr auto locationIndicatorShaderPrelude = R"(#define idLocationIndicatorDrawableUBO  drawableUBOStartId)";

template <>
struct ShaderSource<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LocationIndicatorShader";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = locationIndicatorShaderPrelude;
    static constexpr auto vertex = R"(
layout(location = 0) in vec2 in_position;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

layout(std140, set = LAYER_SET_INDEX, binding = idProjectionUBO) readonly buffer ProjectionUBOVector {
    ProjectionUBO projection_ubo[];
} projectionVector;

void main() {
    gl_Position = projectTile(in_position, projectionVector.projection_ubo[constant.ubo_index]);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) out vec4 out_color;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idLocationIndicatorDrawableUBO) uniform LocationIndicatorDrawableUBO {
    mat4 matrix;
    vec4 color;
} drawable;

void main() {
    out_color = drawable.color;
}
)";
};

template <>
struct ShaderSource<BuiltIn::LocationIndicatorTexturedShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LocationIndicatorTexturedShader";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = locationIndicatorShaderPrelude;
    static constexpr auto vertex = R"(
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

layout(std140, set = LAYER_SET_INDEX, binding = idProjectionUBO) readonly buffer ProjectionUBOVector {
    ProjectionUBO projection_ubo[];
} projectionVector;

layout(location = 0) out vec2 frag_uv;

void main() {
    gl_Position = projectTile(in_position, projectionVector.projection_ubo[constant.ubo_index]);
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
} // namespace mln
