// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "HeatmapTextureShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;
out vec2 v_pos;

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

layout (std140) uniform HeatmapTexturePropsUBO {
    highp mat4 u_matrix;
    highp float u_opacity;
    lowp float props_pad1;
    lowp float props_pad2;
    lowp float props_pad3;
};

void main() {
    gl_Position = u_matrix * vec4(a_pos * u_world_size, 0, 1);

    v_pos.x = a_pos.x;
    v_pos.y = 1.0 - a_pos.y;
}
)";
    static constexpr const char* fragment = R"(in vec2 v_pos;
uniform sampler2D u_image;
uniform sampler2D u_color_ramp;

layout (std140) uniform HeatmapTexturePropsUBO {
    highp mat4 u_matrix;
    highp float u_opacity;
    lowp float props_pad1;
    lowp float props_pad2;
    lowp float props_pad3;
};

void main() {
    float t = texture(u_image, v_pos).r;
    vec4 color = texture(u_color_ramp, vec2(t, 0.5));
    fragColor = color * u_opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(0.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
