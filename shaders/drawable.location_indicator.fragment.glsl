layout (std140) uniform LocationIndicatorDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
};

void main() {
    fragColor = u_color;
}
