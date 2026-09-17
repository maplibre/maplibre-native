layout (std140) uniform LocationIndicatorDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
    vec4 u_sector;
};

layout(location = 0) in vec2 a_pos;

out vec2 v_local;

void main() {
    v_local = a_pos;
    gl_Position = u_matrix * vec4(a_pos, 0.0, 1.0);
}
