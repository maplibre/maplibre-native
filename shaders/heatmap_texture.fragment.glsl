in vec2 v_pos;
uniform sampler2D u_image;
uniform sampler2D u_color_ramp;

layout (std140) uniform HeatmapTexturePropsUBO {
    highp mat4 u_matrix;
    highp float u_opacity;
    lowp float props_pad1;
    lowp float props_pad2;
    lowp float props_pad3;
};

void main() {
    float t = texture(u_image, v_pos).r;
    vec4 color = texture(u_color_ramp, vec2(t, 0.5));
    fragColor = color * u_opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(0.0);
#endif
}
