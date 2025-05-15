layout (std140) uniform DebugUBO {
    highp mat4 u_matrix;
    highp vec4 u_color;
    highp float u_overlay_scale;
    lowp float pad1;
    lowp float pad2;
    lowp float pad3;
};

uniform sampler2D u_overlay;

in vec2 v_uv;

void main() {
    vec4 overlay_color = texture(u_overlay, v_uv);
    fragColor = mix(u_color, overlay_color, overlay_color.a);
}
