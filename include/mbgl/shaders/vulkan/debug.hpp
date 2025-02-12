#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto debugShaderPrelude = R"(#define idDebugUBO  drawableReservedUBOCount)";

template <>
struct ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "DebugShader";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = debugShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idDebugUBO) uniform DebugUBO {
    mat4 matrix;
    vec4 color;
    float overlay_scale;
    float pad1;
    float pad2;
    float pad3;
} debug;

layout(location = 0) out vec2 frag_uv;

void main() {

    gl_Position = debug.matrix * vec4(in_position * debug.overlay_scale, 0, 1);
    applySurfaceTransform();

    // This vertex shader expects a EXTENT x EXTENT quad,
    // The UV co-ordinates for the overlay texture can be calculated using that knowledge
    frag_uv = in_position / 8192.0;
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) in vec2 frag_uv;
layout(location = 0) out vec4 out_color;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idDebugUBO) uniform DebugUBO {
    mat4 matrix;
    vec4 color;
    float overlay_scale;
    float pad1;
    float pad2;
    float pad3;
} debug;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image_sampler;

void main() {
    vec4 overlay_color = texture(image_sampler, frag_uv);
    out_color = mix(debug.color, overlay_color, overlay_color.a);
}
)";
};

} // namespace shaders
} // namespace mbgl
