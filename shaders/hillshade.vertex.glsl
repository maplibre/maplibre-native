layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_pos;

layout (std140) uniform HillshadeDrawableUBO {
    highp mat4 u_matrix;
};

out vec2 v_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    v_pos = a_texture_pos / 8192.0;
}
