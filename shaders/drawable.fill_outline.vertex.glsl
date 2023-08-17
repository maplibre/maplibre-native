layout (std140) uniform FillInterpolateUBO {
    highp float u_color_t;
    highp float u_opacity_t;
    highp float u_outline_color_t;
    highp float u_pattern_from_t;
    highp float u_pattern_to_t;
    highp float u_fade;
    highp float u_padding_interp1;
    highp float u_padding_interp2;
};
layout (std140) uniform FillDrawableUBO {
    highp mat4 u_matrix;
    highp vec4 u_scale;
    highp vec2 u_world;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp vec2 u_texsize;
};
layout (std140) uniform FillDrawablePropsUBO {
    highp vec4 u_color;
    highp vec4 u_outline_color;
    highp float u_opacity;
    highp float padding_props1;
    highp float padding_props2;
    highp float padding_props3;
};
layout (std140) uniform FillDrawableTilePropsUBO {
    highp vec4 u_pattern_from;
    highp vec4 u_pattern_to;
};

layout (location = 0) in vec2 a_pos;

out vec2 v_pos;

#pragma mapbox: define highp vec4 outline_color
#pragma mapbox: define lowp float opacity

void main() {
    #pragma mapbox: initialize highp vec4 outline_color
    #pragma mapbox: initialize lowp float opacity

    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    v_pos = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * u_world;
}
