layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_pos;

layout (std140) uniform HillshadeDrawableUBO {
    highp mat4 u_matrix;
};

uniform sampler2D u_image;

out vec2 v_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    // The prepare-pass renders a (dim+3)² texture: one "ghost" texel
    // on each side beyond the displayed tile area, whose Sobel inputs
    // are all backfilled from the matching neighbour. Inset v_pos so
    // the displayed tile [world 0, dim] maps to the inner dim+1
    // texels (indices 1 to dim+1), with the outer ring providing
    // shared-data bilinear partners at the boundary.
    float texW = float(textureSize(u_image, 0).x);
    float scale = (texW - 3.0) / texW;
    float offset = 1.0 / texW;
    v_pos = (a_texture_pos / 8192.0) * scale + offset;
}
