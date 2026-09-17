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
layout(location = 0) out vec2 frag_local;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idLocationIndicatorDrawableUBO) uniform LocationIndicatorDrawableUBO {
    mat4 matrix;
    vec4 color;
    vec4 sector;
} drawable;

void main() {
    frag_local = in_position;
    gl_Position = drawable.matrix * vec4(in_position, 0, 1);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) out vec4 out_color;
layout(location = 0) in vec2 frag_local;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idLocationIndicatorDrawableUBO) uniform LocationIndicatorDrawableUBO {
    mat4 matrix;
    vec4 color;
    vec4 sector;
} drawable;

void main() {
    float opacity = 1.0;
    if (drawable.sector.y > 0.0) {
        float radius = length(frag_local);
        float angle = atan(max(abs(frag_local.x), 0.000001), -frag_local.y);
        float feather = length(fwidth(frag_local)) / max(radius, 0.0001);
        float angular = drawable.sector.x >= 3.14159265 ? 1.0 :
            1.0 - smoothstep(drawable.sector.x - feather, drawable.sector.x + feather, angle);
        opacity = angular * (1.0 - smoothstep(0.0, 1.0, radius));
    }
    out_color = drawable.color * opacity;
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
    vec4 sector;
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
    // using lod bias variant for sampling as a workaround for Adreno 600 series
    out_color = texture(image_sampler, frag_uv, 0.0);
    // working alternatives
    //out_color = textureGrad(image_sampler, frag_uv, dFdx(frag_uv), dFdy(frag_uv));
    //out_color = texelFetch(image_sampler, ivec2(frag_uv * vec2(textureSize(image_sampler, 0))), 0);
}
)";
};

} // namespace shaders
} // namespace mln
