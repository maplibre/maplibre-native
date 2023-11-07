layout (std140) uniform DebugUBO {
    highp mat4 u_matrix;
    highp vec4 u_color;
    highp float u_overlay_scale;
    highp float pad1;
    highp float pad2;
    highp float pad3;
};

uniform sampler2D u_overlay;

in vec2 v_uv;

void main() {
    vec4 overlay_color = texture(u_overlay, v_uv);
    fragColor = mix(u_color, overlay_color, overlay_color.a);
}
