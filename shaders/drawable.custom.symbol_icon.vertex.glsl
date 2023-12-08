layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex;

layout (std140) uniform SymbolDrawableUBO {
    highp mat4 u_matrix;
};

out vec2 v_tex;

void main() {
    v_tex = a_tex;
    gl_Position = u_matrix * vec4(a_pos, 0.0, 1.0);
}
