// Generates backend sources from one polygon/projection algorithm.
// Prints the header that is checked in as shared/cpp/*_shader_sources.hpp.

import { shaderHeader, type PointShader } from '../../../tools/shaders.ts';

// The paint properties: the name, the type, and the attribute ids of the values at the lower and upper zoom.
const properties: PointShader['properties'] = [
  ['radius', 'float', 1, 1], ['corners', 'float', 2, 2], ['rotate', 'float', 3, 3],
  ['color', 'vec4', 4, 5], ['blur', 'float', 6, 6], ['opacity', 'float', 7, 7],
  ['stroke_width', 'float', 8, 8], ['stroke_color', 'vec4', 9, 10],
  ['stroke_opacity', 'float', 11, 11], ['translate', 'vec2', 12, 12],
  ['translate_anchor', 'enum', 13, 13], ['pitch_alignment', 'enum', 14, 14],
  ['pitch_scale', 'enum', 15, 15],
];
const uniformFields = `
    mat4 matrix;
    vec4 camera;
    vec4 view;
    float radius;
    float corners;
    float rotate;
    float blur;
    float opacity;
    float stroke_width;
    float stroke_opacity;
    float translate_anchor;
    vec4 color;
    vec4 stroke_color;
    vec2 translate;
    float pitch_alignment;
    float pitch_scale;
    vec4 interpolation0;
    vec4 interpolation1;
    vec4 interpolation2;
    vec4 interpolation3;
    mat4 projection;
    vec4 tile_mercator;
    vec4 clipping_plane;
    vec4 globe;
`;
const varyings: PointShader['varyings'] = [['vec2', 'local'], ['vec4', 'shape'], ['vec4', 'paint'], ['vec4', 'color'], ['vec4', 'stroke_color']];

// The globe, as the host's own shaders project it. `globe` is the transition
// and the angle of a pixel on the sphere.
const functions = `
vec3 ngon_sphere(vec2 tile_position, vec4 tile_mercator) {
    vec2 mercator = tile_mercator.xy + tile_mercator.zw * tile_position;
    float x = mercator.x * 6.283185307179586 + 3.141592653589793;
    float t = exp(3.141592653589793 - mercator.y * 6.283185307179586);
    return vec3(sin(x) * 2.0 * t, t * t - 1.0, cos(x) * 2.0 * t) / (t * t + 1.0);
}

vec3 ngon_rotate(vec3 direction, vec2 angles) {
    vec3 right = vec3(direction.z, 0.0, -direction.x);
    vec3 up = normalize(cross(right, direction));
    vec2 t = tan(angles);
    return normalize(direction + normalize(right) * t.x + up * t.y);
}

vec4 ngon_project(vec2 tile_position, vec3 direction, mat4 tile_matrix, mat4 projection, vec4 plane, vec4 globe) {
    vec4 result = projection * vec4(direction, 1.0);
    result.z = (1.0 - (dot(direction, plane.xyz) + plane.w)) * result.w;
    if (globe.x > 0.999) return result;
    vec4 flat_position = tile_matrix * vec4(tile_position, 0.0, 1.0);
    result.z = mix(0.0, result.z, clamp((globe.x - 0.2) / 0.8, 0.0, 1.0));
    result.xyw = mix(flat_position.xyw, result.xyw, globe.x);
    return result;
}
`;

// Pixel coordinates have positive y downwards. Corner zero points up; rotation
// is clockwise. A constant quad allows even corner count to be feature-driven.
const projection = `
    radius = max(radius, 0.0);
    stroke_width = max(stroke_width, 0.0);
    corners = clamp(floor(corners + 0.5), 3.0, 360.0);
    vec2 encoded = vec2(POSITION);
    vec2 center = floor(encoded * 0.5);
    vec2 corner = encoded - 2.0 * center;
    corner = corner * 2.0 - 1.0;
    if (translate_anchor > 0.5) {
        float c = cos(-u.view.x), s = sin(-u.view.x);
        translate = vec2(c * translate.x - s * translate.y, s * translate.x + c * translate.y);
    }
    center += translate * u.camera.z;
    bool on_globe = u.globe.x > 0.0;
    vec3 center_vector = ngon_sphere(center, u.tile_mercator);
    vec4 projected = u.matrix * vec4(center, 0.0, 1.0);
    if (on_globe) projected = ngon_project(center, center_vector, u.matrix, u.projection, u.clipping_plane, u.globe);
    float outer_radius = radius + stroke_width / cos(3.141592653589793 / corners);
    float extent = outer_radius + 2.0 / max(u.view.y, 1.0);
    vec2 local = corner * extent;
    vec4 position;
    if (pitch_alignment < 0.5) {
        vec2 offset = local * u.camera.z;
        if (pitch_scale > 0.5) offset *= projected.w / u.camera.w;
        position = u.matrix * vec4(center + offset, 0.0, 1.0);
        if (on_globe) {
            vec2 angles = offset * (u.globe.y / u.camera.z);
            position = ngon_project(center + offset, ngon_rotate(center_vector, angles), u.matrix, u.projection,
                                    u.clipping_plane, u.globe);
        }
    } else {
        float scale = pitch_scale < 0.5 ? u.camera.w : projected.w;
        position = projected + vec4(local * u.camera.xy * scale, 0.0, 0.0);
        // Behind the horizon.
        if (on_globe && projected.z > projected.w) position.xy = vec2(10000.0);
    }
    OUT_local = local;
    OUT_shape = vec4(radius, stroke_width, corners, rotate * 0.017453292519943295);
    OUT_paint = vec4(clamp(blur, 0.0, 1.0), clamp(opacity, 0.0, 1.0), clamp(stroke_opacity, 0.0, 1.0), 0.0);
    OUT_color = color;
    OUT_stroke_color = stroke_color;
`;

const coverage = `
    float radius = IN_shape.x;
    float stroke = IN_shape.y;
    float sector = 6.283185307179586 / IN_shape.z;
    float angle = (length(IN_local) > 0.0 ? atan(IN_local.y, IN_local.x) : 0.0) + 1.5707963267948966 - IN_shape.w;
    float edge = floor(angle / sector) * sector + sector * 0.5;
    float support = cos(angle - edge) * length(IN_local);
    float inner_distance = radius * cos(sector * 0.5) - support;
    float aa = max(fwidth(support) * 0.5, 0.0001);
    float outer = smoothstep(-aa, max(aa, IN_paint.x * radius), inner_distance + stroke);
    float fill = stroke > 0.0 ? smoothstep(-aa, aa, inner_distance) : 1.0;
    vec4 result = mix(IN_stroke_color * IN_paint.z, IN_color * IN_paint.y, fill) * outer;
    if (radius + stroke <= 0.0) result = vec4(0.0);
`;

process.stdout.write(shaderHeader({ name: 'ngon', properties, uniformFields, varyings, functions, projection, coverage }));
