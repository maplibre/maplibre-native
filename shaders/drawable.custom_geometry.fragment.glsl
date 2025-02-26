layout (std140) uniform CustomGeometryDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
};

in vec2 frag_uv;
uniform sampler2D u_image;

void main() {
    fragColor = texture(u_image, frag_uv) * u_color;
}
