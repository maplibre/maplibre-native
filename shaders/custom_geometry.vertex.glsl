layout (std140) uniform CustomGeometryDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
};

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 frag_uv;

void main() {
    frag_uv = a_uv;
    gl_Position = u_matrix * vec4(a_pos, 1.0);
}
