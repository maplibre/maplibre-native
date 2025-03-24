#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto customSymbolIconShaderPrelude = R"(#define idCustomSymbolDrawableUBO idDrawableReservedVertexOnlyUBO)";

template <>
struct ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "CustomSymbolIconShader";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = customSymbolIconShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_tex;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idCustomSymbolDrawableUBO) uniform CustomSymbolIconDrawableUBO {
    mat4 matrix;
    vec2 extrude_scale;
    vec2 anchor;
    float angle_degrees;
    bool scale_with_map;
    bool pitch_with_map;
    float camera_to_center_distance;
    float aspect_ratio;
    float pad1;
    float pad2;
    float pad3;
} drawable;

layout(location = 0) out vec2 frag_tex;

vec2 rotateVec2(vec2 v, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    return vec2(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}

vec2 ellipseRotateVec2(vec2 v, float angle, float radiusRatio /* A/B */) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    float invRatio = 1.0 / radiusRatio;
    return vec2(v.x * cosA - radiusRatio * v.y * sinA, invRatio * v.x * sinA + v.y * cosA);
}

void main() {
    const vec2 extrude = mod(in_position, 2.0) * 2.0 - 1.0;
    const vec2 anchor = (drawable.anchor - vec2(0.5, 0.5)) * 2.0;
    const vec2 center = floor(in_position * 0.5);
    const float angle = radians(-drawable.angle_degrees);
    vec2 corner = extrude - anchor;

    vec4 position;
    if (drawable.pitch_with_map) {
        if (drawable.scale_with_map) {
            corner *= drawable.extrude_scale;
        } else {
            vec4 projected_center = drawable.matrix * vec4(center, 0, 1);
            corner *= drawable.extrude_scale * (projected_center.w / drawable.camera_to_center_distance);
        }
        corner = center + rotateVec2(corner, angle);
        position = drawable.matrix * vec4(corner, 0, 1);
    } else {
        position = drawable.matrix * vec4(center, 0, 1);
        const float factor = drawable.scale_with_map ? drawable.camera_to_center_distance : position.w;
        position.xy += ellipseRotateVec2(corner * drawable.extrude_scale * factor, angle, drawable.aspect_ratio);
    }

    gl_Position = position;
    applySurfaceTransform();

    frag_tex = in_tex;
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) in vec2 frag_tex;
layout(location = 0) out vec4 out_color;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    out_color = texture(image_sampler, frag_tex);
}
)";
};

} // namespace shaders
} // namespace mbgl
