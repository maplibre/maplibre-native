layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex;

layout (std140) uniform SymbolDrawableUBO {
    highp mat4 u_matrix;
};

layout (std140) uniform SymbolParametersUBO {
    highp vec2 u_extrude_scale;
    highp vec2 u_anchor;
    highp float u_angle_degrees;
    highp float pad0, pad1, pad2;
};

out vec2 v_tex;

vec2 rotateVec2(vec2 v, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    return vec2(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}

void main() {
    // unencode the extrusion vector (-1, -1) to (1, 1)
    vec2 extrude = vec2(mod(a_pos, 2.0) * 2.0 - 1.0);

    // anchor in the range (-1, -1) to (1, 1)
    vec2 anchor = (u_anchor - vec2(0.5, 0.5)) * 2.0;

    // get center
    vec2 center = floor(a_pos * 0.5);

    gl_Position = u_matrix * vec4(center, 0, 1);
    float angle = radians(u_angle_degrees);
    gl_Position.xy += rotateVec2((extrude - anchor) * u_extrude_scale * DEVICE_PIXEL_RATIO, angle);

    v_tex = a_tex;
}
