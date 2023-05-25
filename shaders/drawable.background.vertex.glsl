layout (location = 0) in vec2 a_pos;
layout (std140) uniform BackgroundDrawableUBO {
    mat4 u_matrix;
    vec2 u_world;
    vec2 pad;
};

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
