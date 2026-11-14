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
`;
const varyings: PointShader['varyings'] = [['vec2', 'local'], ['vec4', 'shape'], ['vec4', 'paint'], ['vec4', 'color'], ['vec4', 'stroke_color']];

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
    vec4 projected = u.matrix * vec4(center, 0.0, 1.0);
    float outer_radius = radius + stroke_width / cos(3.141592653589793 / corners);
    float extent = outer_radius + 2.0 / max(u.view.y, 1.0);
    vec2 local = corner * extent;
    vec4 position;
    if (pitch_alignment < 0.5) {
        vec2 offset = local * u.camera.z;
        if (pitch_scale > 0.5) offset *= projected.w / u.camera.w;
        position = u.matrix * vec4(center + offset, 0.0, 1.0);
    } else {
        float scale = pitch_scale < 0.5 ? u.camera.w : projected.w;
        position = projected + vec4(local * u.camera.xy * scale, 0.0, 0.0);
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

process.stdout.write(shaderHeader({ name: 'ngon', properties, uniformFields, varyings, projection, coverage }));
