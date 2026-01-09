layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex;

layout(std140) uniform CustomSymbolIconDrawableUBO {
    highp mat4 u_matrix;
    highp vec2 u_extrude_scale;
    highp vec2 u_anchor;
    highp float u_angle_degrees;
    bool u_scale_with_map;
    bool u_pitch_with_map;
    highp float u_camera_to_center_distance;
    highp float u_aspect_ratio;
    lowp float drawable_pad1;
    lowp float drawable_pad2;
    lowp float drawable_pad3;
};

out vec2 v_tex;

vec2 rotateVec2(vec2 v, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    return vec2(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}

vec2 ellipseRotateVec2(vec2 v, float angle, float radiusRatio /* A/B */) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    float invRatio = 1.0 / radiusRatio;
    return vec2(v.x * cosA - radiusRatio * v.y * sinA, invRatio * v.x * sinA + v.y * cosA);
}

void main() {
    // decode the extrusion vector (-1, -1) to (1, 1)
    vec2 extrude = vec2(mod(a_pos, 2.0) * 2.0 - 1.0);

    // make anchor relative to (0.5, 0.5) and corners in range (-1, -1) to (1, 1)
    vec2 anchor = (u_anchor - vec2(0.5, 0.5)) * 2.0;

    // decode center
    vec2 center = floor(a_pos * 0.5);

    // rotate extrusion around anchor
    float angle = radians(-u_angle_degrees);
    vec2 corner = extrude - anchor;

    // compute
    if (u_pitch_with_map) {
        if (u_scale_with_map) {
            corner *= u_extrude_scale;
        } else {
            vec4 projected_center = u_matrix * vec4(center, 0, 1);
            corner *= u_extrude_scale * (projected_center.w / u_camera_to_center_distance);
        }
        corner = center + rotateVec2(corner, angle);
        gl_Position = u_matrix * vec4(corner, 0, 1);
    } else {
        gl_Position = u_matrix * vec4(center, 0, 1);
        if (u_scale_with_map) {
            gl_Position.xy += ellipseRotateVec2(corner * u_extrude_scale * u_camera_to_center_distance, angle, u_aspect_ratio);
        } else {
            gl_Position.xy += ellipseRotateVec2(corner * u_extrude_scale * gl_Position.w, angle, u_aspect_ratio);
        }
    }

    // texture coordinates
    v_tex = a_tex;
}
