// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>


namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "BackgroundPatternShader";


    static constexpr const char* vertexData = R"(layout (std140) uniform BackgroundDrawableUBO {
    highp mat4 u_matrix;
};
layout (std140) uniform BackgroundLayerUBO {
    highp vec2 u_pattern_tl_a;
    highp vec2 u_pattern_br_a;
    highp vec2 u_pattern_tl_b;
    highp vec2 u_pattern_br_b;
    highp vec2 u_texsize;
    highp vec2 u_pattern_size_a;
    highp vec2 u_pattern_size_b;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp float u_tile_units_to_pixels;
    highp float u_scale_a;
    highp float u_scale_b;
    highp float u_mix;
    highp float u_opacity;
    highp float pad;
};

layout (location = 0) in vec2 a_pos;
out mediump vec2 v_pos_a;
out mediump vec2 v_pos_b;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);

    v_pos_a = get_pattern_pos(u_pixel_coord_upper, u_pixel_coord_lower, u_scale_a * u_pattern_size_a, u_tile_units_to_pixels, a_pos);
    v_pos_b = get_pattern_pos(u_pixel_coord_upper, u_pixel_coord_lower, u_scale_b * u_pattern_size_b, u_tile_units_to_pixels, a_pos);
}
)";
    static constexpr const char* fragmentData = R"(layout (std140) uniform BackgroundLayerUBO {
    highp vec2 u_pattern_tl_a;
    highp vec2 u_pattern_br_a;
    highp vec2 u_pattern_tl_b;
    highp vec2 u_pattern_br_b;
    highp vec2 u_texsize;
    highp vec2 u_pattern_size_a;
    highp vec2 u_pattern_size_b;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp float u_tile_units_to_pixels;
    highp float u_scale_a;
    highp float u_scale_b;
    highp float u_mix;
    highp float u_opacity;
    highp float pad;
};

uniform sampler2D u_image;

in mediump vec2 v_pos_a;
in mediump vec2 v_pos_b;

void main() {
    vec2 imagecoord = mod(v_pos_a, 1.0);
    vec2 pos = mix(u_pattern_tl_a / u_texsize, u_pattern_br_a / u_texsize, imagecoord);
    vec4 color1 = texture(u_image, pos);

    vec2 imagecoord_b = mod(v_pos_b, 1.0);
    vec2 pos2 = mix(u_pattern_tl_b / u_texsize, u_pattern_br_b / u_texsize, imagecoord_b);
    vec4 color2 = texture(u_image, pos2);

    fragColor = mix(color1, color2, u_mix) * u_opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
    static std::string vertex() {
        using Ty = ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL>;
        return Ty::vertexData;
    }
    static std::string fragment() {
        using Ty = ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL>;
        return Ty::fragmentData;
    }
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
