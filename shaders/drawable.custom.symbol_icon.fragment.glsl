uniform sampler2D u_texture;

in vec2 v_tex;

void main() {
    fragColor = texture(u_texture, v_tex);
}
