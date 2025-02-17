layout (std140) uniform LocationIndicatorDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
};

layout(location = 0) in vec2 a_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0.0, 1.0);
}
