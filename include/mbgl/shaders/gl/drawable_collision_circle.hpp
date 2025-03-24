// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "CollisionCircleShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_anchor_pos;
layout (location = 2) in vec2 a_extrude;
layout (location = 3) in vec2 a_placed;

layout (std140) uniform GlobalPaintParamsUBO {
    highp vec2 u_pattern_atlas_texsize;
    highp vec2 u_units_to_pixels;
    highp vec2 u_world_size;
    highp float u_camera_to_center_distance;
    highp float u_symbol_fade_change;
    highp float u_aspect_ratio;
    highp float u_pixel_ratio;
    highp float u_map_zoom;
    lowp float global_pad1;
};

layout (std140) uniform CollisionDrawableUBO {
    highp mat4 u_matrix;
};

layout (std140) uniform CollisionTilePropsUBO {
    highp vec2 u_extrude_scale;
    highp float u_overscale_factor;
    lowp float drawable_pad1;
};

out float v_placed;
out float v_notUsed;
out float v_radius;
out highp vec2 v_extrude;
out vec2 v_extrude_scale;

void main() {
    vec4 projectedPoint = u_matrix * vec4(a_anchor_pos, 0, 1);
    highp float camera_to_anchor_distance = projectedPoint.w;
    highp float collision_perspective_ratio = clamp(
        0.5 + 0.5 * (u_camera_to_center_distance / camera_to_anchor_distance),
        0.0, // Prevents oversized near-field circles in pitched/overzoomed tiles
        4.0);

    gl_Position = u_matrix * vec4(a_pos, 0.0, 1.0);

    highp float padding_factor = 1.2; // Pad the vertices slightly to make room for anti-alias blur
    gl_Position.xy += a_extrude * u_extrude_scale * padding_factor * gl_Position.w * collision_perspective_ratio;

    v_placed = a_placed.x;
    v_notUsed = a_placed.y;
    v_radius = abs(a_extrude.y); // We don't pitch the circles, so both units of the extrusion vector are equal in magnitude to the radius

    v_extrude = a_extrude * padding_factor;
    v_extrude_scale = u_extrude_scale * u_camera_to_center_distance * collision_perspective_ratio;
}
)";
    static constexpr const char* fragment = R"(layout (std140) uniform GlobalPaintParamsUBO {
    highp vec2 u_pattern_atlas_texsize;
    highp vec2 u_units_to_pixels;
    highp vec2 u_world_size;
    highp float u_camera_to_center_distance;
    highp float u_symbol_fade_change;
    highp float u_aspect_ratio;
    highp float u_pixel_ratio;
    highp float u_map_zoom;
    lowp float global_pad1;
};

layout (std140) uniform CollisionTilePropsUBO {
    highp vec2 u_extrude_scale;
    highp float u_overscale_factor;
    lowp float drawable_pad1;
};

in float v_placed;
in float v_notUsed;
in float v_radius;
in highp vec2 v_extrude;
in vec2 v_extrude_scale;

void main() {
    float alpha = 0.5;

    // Red = collision, hide label
    vec4 color = vec4(1.0, 0.0, 0.0, 1.0) * alpha;

    // Blue = no collision, label is showing
    if (v_placed > 0.5) {
        color = vec4(0.0, 0.0, 1.0, 0.5) * alpha;
    }

    if (v_notUsed > 0.5) {
        // This box not used, fade it out
        color *= .2;
    }

    float extrude_scale_length = length(v_extrude_scale);
    float extrude_length = length(v_extrude) * extrude_scale_length;
    float stroke_width = 15.0 * extrude_scale_length / u_overscale_factor;
    float radius = v_radius * extrude_scale_length;

    float distance_to_edge = abs(extrude_length - radius);
    float opacity_t = smoothstep(-stroke_width, 0.0, -distance_to_edge);

    fragColor = opacity_t * color;
}
)";
};

} // namespace shaders
} // namespace mbgl
