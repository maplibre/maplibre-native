#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto locationIndicatorShaderPrelude = R"(#define idLocationIndicatorDrawableUBO  drawableReservedUBOCount)";

template <>
struct ShaderSource<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LocationIndicatorShader";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = locationIndicatorShaderPrelude;
    static constexpr auto vertex = R"(
layout(location = 0) in vec2 in_position;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idLocationIndicatorDrawableUBO) uniform LocationIndicatorDrawableUBO {
    mat4 matrix;
    vec4 color;
} drawable;

void main() {
    gl_Position = drawable.matrix * vec4(in_position, 0, 1);
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

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idLocationIndicatorDrawableUBO) uniform LocationIndicatorDrawableUBO {
    mat4 matrix;
    vec4 color;
} drawable;

layout(location = 0) out vec2 frag_uv;

void main() {
    gl_Position = drawable.matrix * vec4(in_position, 0, 1);
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
