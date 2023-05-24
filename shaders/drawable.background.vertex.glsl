layout (location = 0) in vec2 a_pos;
layout (std140) uniform DrawableUBO {
    highp mat4 u_matrix;
    highp vec2 u_world;
    highp vec2 pad;
};

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
