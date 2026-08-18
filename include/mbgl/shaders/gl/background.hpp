// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "BackgroundShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;
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
    highp vec4 u_drape_tile;
};

layout (std140) uniform BackgroundDrawableUBO {
    highp mat4 u_matrix;
};

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    gl_Position = apply_drape_transform(gl_Position, u_matrix, u_drape_tile);
}
)";
    static constexpr const char* fragment = R"(layout (std140) uniform BackgroundPropsUBO {
    highp vec4 u_color;
    highp float u_opacity;
    lowp float props_pad1;
    lowp float props_pad2;
    lowp float props_pad3;
};

void main() {
    fragColor = u_color * u_opacity;
#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
