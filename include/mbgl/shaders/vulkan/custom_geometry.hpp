#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto customGeometryShaderPrelude = R"(#define idCustomGeometryDrawableUBO  drawableReservedUBOCount)";

template <>
struct ShaderSource<BuiltIn::CustomGeometryShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "CustomGeometryShader";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = customGeometryShaderPrelude;
    static constexpr auto vertex = R"(
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idCustomGeometryDrawableUBO) uniform CustomDrawableDrawableUBO {
    mat4 matrix;
    vec4 color;
} drawable;

layout(location = 0) out vec2 frag_uv;

void main() {
    frag_uv = in_texcoord;

    gl_Position = drawable.matrix * vec4(in_position, 1.0);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) in vec2 frag_uv;
layout(location = 0) out vec4 out_color;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idCustomGeometryDrawableUBO) uniform CustomDrawableDrawableUBO {
    mat4 matrix;
    vec4 color;
} drawable;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image_sampler;

void main() {
    out_color = texture(image_sampler, frag_uv) * drawable.color;
}
)";
};

} // namespace shaders
} // namespace mbgl
