layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_pos;

layout (std140) uniform HillshadePrepareDrawableUBO {
    highp mat4 u_matrix;
};

layout (std140) uniform HillshadePrepareTilePropsUBO {
    highp vec4 u_unpack;
    highp vec2 u_dimension;
    highp float u_zoom;
    highp float u_maxzoom;
    highp float u_log2_world_width;
    lowp float tileprops_pad1;
    lowp float tileprops_pad2;
    lowp float tileprops_pad3;
};

out vec2 v_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);

    highp vec2 epsilon = 1.0 / u_dimension;
    float scale = (u_dimension.x - 2.0) / u_dimension.x;
    v_pos = (a_texture_pos / 8192.0) * scale + epsilon;
}
