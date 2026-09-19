layout (std140) uniform LocationIndicatorDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
    vec4 u_sector;
};

in vec2 frag_uv;
uniform sampler2D u_image;

void main() {
    fragColor = texture(u_image, frag_uv);
}
