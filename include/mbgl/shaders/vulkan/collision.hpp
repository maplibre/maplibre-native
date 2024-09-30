#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "CollisionBoxShader";

    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec2 in_anchor_position;
layout(location = 2) in ivec2 in_extrude;
layout(location = 3) in uvec2 in_placed;
layout(location = 4) in vec2 in_shift;

layout(set = 0, binding = 1) uniform CollisionBoxUBO {
    mat4 matrix;
    vec2 extrude_scale;
    float overscale_factor;
    float pad1;
} drawable;

layout(location = 0) out float frag_placed;
layout(location = 1) out float frag_notUsed;

void main() {

    vec4 projectedPoint = drawable.matrix * vec4(in_anchor_position, 0.0, 1.0);
    float camera_to_anchor_distance = projectedPoint.w;
    float collision_perspective_ratio = clamp(
        0.5 + 0.5 * (global.camera_to_center_distance / camera_to_anchor_distance),
        0.0, // Prevents oversized near-field boxes in pitched/overzoomed tiles
        4.0);

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    gl_Position.xy += (in_extrude + in_shift) * drawable.extrude_scale * gl_Position.w * collision_perspective_ratio;
    gl_Position.y *= -1.0;

    frag_placed = in_placed.x;
    frag_notUsed = in_placed.y;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in float frag_placed;
layout(location = 1) in float frag_notUsed;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform CollisionBoxUBO {
    mat4 matrix;
    vec2 extrude_scale;
    float overscale_factor;
    float pad1;
} drawable;

void main() {

    float alpha = 0.5;

    // Red = collision, hide label
    vec4 color = vec4(1.0, 0.0, 0.0, 1.0) * alpha;

    // Blue = no collision, label is showing
    if (frag_placed > 0.5) {
        color = vec4(0.0, 0.0, 1.0, 0.5) * alpha;
    }

    if (frag_notUsed > 0.5) {
        // This box not used, fade it out
        color *= 0.1;
    }

    out_color = color;
}
)";
};

template <>
struct ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "CollisionCircleShader";

    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec2 in_anchor_position;
layout(location = 2) in ivec2 in_extrude;
layout(location = 3) in uvec2 in_placed;

layout(set = 0, binding = 1) uniform CollisionCircleUBO {
    mat4 matrix;
    vec2 extrude_scale;
    float overscale_factor;
    float pad1;
} drawable;

layout(location = 0) out float frag_placed;
layout(location = 1) out float frag_notUsed;
layout(location = 2) out float frag_radius;
layout(location = 3) out vec2 frag_extrude;
layout(location = 4) out vec2 frag_extrude_scale;

void main() {

    vec4 projectedPoint = drawable.matrix * vec4(in_anchor_position, 0, 1);
    float camera_to_anchor_distance = projectedPoint.w;
    float collision_perspective_ratio = clamp(
        0.5 + 0.5 * (global.camera_to_center_distance / camera_to_anchor_distance),
        0.0, // Prevents oversized near-field circles in pitched/overzoomed tiles
        4.0);

    float padding_factor = 1.2; // Pad the vertices slightly to make room for anti-alias blur
    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    gl_Position.xy += in_extrude * drawable.extrude_scale * padding_factor * gl_Position.w * collision_perspective_ratio;
    gl_Position.y *= -1.0;

    frag_placed = in_placed.x;
    frag_notUsed = in_placed.y;
    frag_radius = abs(float(in_extrude.y)); // We don't pitch the circles, so both units of the extrusion vector are equal in magnitude to the radius
    frag_extrude = in_extrude * padding_factor;
    frag_extrude_scale = drawable.extrude_scale * global.camera_to_center_distance * collision_perspective_ratio;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in float frag_placed;
layout(location = 1) in float frag_notUsed;
layout(location = 2) in float frag_radius;
layout(location = 3) in vec2 frag_extrude;
layout(location = 4) in vec2 frag_extrude_scale;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform CollisionCircleUBO {
    mat4 matrix;
    vec2 extrude_scale;
    float overscale_factor;
    float pad1;
} drawable;

void main() {

    float alpha = 0.5;

    // Red = collision, hide label
    vec4 color = vec4(1.0, 0.0, 0.0, 1.0) * alpha;

    // Blue = no collision, label is showing
    if (frag_placed > 0.5) {
        color = vec4(0.0, 0.0, 1.0, 0.5) * alpha;
    }

    if (frag_notUsed > 0.5) {
        // This box not used, fade it out
        color *= 0.2;
    }

    float extrude_scale_length = length(frag_extrude_scale);
    float extrude_length = length(frag_extrude) * extrude_scale_length;
    float stroke_width = 15.0 * extrude_scale_length / drawable.overscale_factor;
    float radius = frag_radius * extrude_scale_length;

    float distance_to_edge = abs(extrude_length - radius);
    float opacity_t = smoothstep(-stroke_width, 0.0, -distance_to_edge);

    out_color = color * opacity_t;
}
)";
};

} // namespace shaders
} // namespace mbgl
