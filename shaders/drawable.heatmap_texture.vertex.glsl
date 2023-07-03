layout (location = 0) in vec2 a_pos;
out vec2 v_pos;

layout (std140) uniform HeatmapTextureDrawableUBO {
    highp mat4 u_matrix;
    highp vec2 u_world;
    highp float u_opacity;
    lowp float pad0_;
};

void main() {
    gl_Position = u_matrix * vec4(a_pos * u_world, 0, 1);

    v_pos.x = a_pos.x;
    v_pos.y = 1.0 - a_pos.y;
}
