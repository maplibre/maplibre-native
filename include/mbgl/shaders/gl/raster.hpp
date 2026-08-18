// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "RasterShader";
    static constexpr const char* vertex = R"(layout (std140) uniform GlobalPaintParamsUBO {
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

layout (std140) uniform RasterDrawableUBO {
    highp mat4 u_matrix;
};
layout (std140) uniform RasterEvaluatedPropsUBO {
    highp vec3 u_spin_weights;
    highp vec2 u_tl_parent;
    highp float u_scale_parent;
    highp float u_buffer_scale;
    highp float u_fade_t;
    highp float u_opacity;
    highp float u_brightness_low;
    highp float u_brightness_high;
    highp float u_saturation_factor;
    highp float u_contrast_factor;
    lowp float props_pad1;
    lowp float props_pad2;
};

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_pos;

out vec2 v_pos0;
out vec2 v_pos1;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    gl_Position = apply_drape_transform(gl_Position, u_matrix, u_drape_tile);
    // We are using Int16 for texture position coordinates to give us enough precision for
    // fractional coordinates. We use 8192 to scale the texture coordinates in the buffer
    // as an arbitrarily high number to preserve adequate precision when rendering.
    // This is also the same value as the EXTENT we are using for our tile buffer pos coordinates,
    // so math for modifying either is consistent.
    v_pos0 = (((a_texture_pos / 8192.0) - 0.5) / u_buffer_scale ) + 0.5;
    v_pos1 = (v_pos0 * u_scale_parent) + u_tl_parent;
}
)";
    static constexpr const char* fragment = R"(layout (std140) uniform RasterEvaluatedPropsUBO {
    highp vec3 u_spin_weights;
    highp vec2 u_tl_parent;
    highp float u_scale_parent;
    highp float u_buffer_scale;
    highp float u_fade_t;
    highp float u_opacity;
    highp float u_brightness_low;
    highp float u_brightness_high;
    highp float u_saturation_factor;
    highp float u_contrast_factor;
    lowp float props_pad1;
    lowp float props_pad2;
};
uniform sampler2D u_image0;
uniform sampler2D u_image1;

in vec2 v_pos0;
in vec2 v_pos1;

void main() {

    // read and cross-fade colors from the main and parent tiles
    vec4 color0 = texture(u_image0, v_pos0);
    vec4 color1 = texture(u_image1, v_pos1);
    if (color0.a > 0.0) {
        color0.rgb = color0.rgb / color0.a;
    }
    if (color1.a > 0.0) {
        color1.rgb = color1.rgb / color1.a;
    }
    vec4 color = mix(color0, color1, u_fade_t);
    color.a *= u_opacity;
    vec3 rgb = color.rgb;

    // spin
    rgb = vec3(
        dot(rgb, u_spin_weights.xyz),
        dot(rgb, u_spin_weights.zxy),
        dot(rgb, u_spin_weights.yzx));

    // saturation
    float average = (color.r + color.g + color.b) / 3.0;
    rgb += (average - rgb) * u_saturation_factor;

    // contrast
    rgb = (rgb - 0.5) * u_contrast_factor + 0.5;

    // brightness
    vec3 u_high_vec = vec3(u_brightness_low, u_brightness_low, u_brightness_low);
    vec3 u_low_vec = vec3(u_brightness_high, u_brightness_high, u_brightness_high);

    fragColor = vec4(mix(u_high_vec, u_low_vec, rgb) * color.a, color.a);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
