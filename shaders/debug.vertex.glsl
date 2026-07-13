layout (location = 0) in vec2 a_pos;
out vec2 v_uv;

layout (std140) uniform DebugUBO {
    highp mat4 u_matrix;
    highp vec4 u_color;
    highp float u_overlay_scale;
    lowp float pad1;
    lowp float pad2;
    lowp float pad3;
};

void main() {
    // This vertex shader expects a EXTENT x EXTENT quad,
    // The UV co-ordinates for the overlay texture can be calculated using that knowledge
    v_uv = a_pos / 8192.0;
    gl_Position = u_matrix * vec4(a_pos * u_overlay_scale, 0, 1);
}
