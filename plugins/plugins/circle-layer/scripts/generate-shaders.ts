// Generates the backend sources of the circle plugin from one algorithm, the built-in circle shader.
// Prints the header that is checked in as shared/cpp/*_shader_sources.hpp.

import { shaderHeader, type PointShader } from '../../../tools/shaders.ts';

// The paint properties: the name, the type, and the attribute ids of the values at the lower and upper zoom.
const properties: PointShader['properties'] = [
  ['radius', 'float', 1, 1], ['color', 'vec4', 2, 3], ['blur', 'float', 4, 4], ['opacity', 'float', 5, 5],
  ['stroke_width', 'float', 6, 6], ['stroke_color', 'vec4', 7, 8], ['stroke_opacity', 'float', 9, 9],
  ['translate', 'vec2', 10, 10], ['translate_anchor', 'enum', 11, 11], ['pitch_alignment', 'enum', 12, 12],
  ['pitch_scale', 'enum', 13, 13],
];
const uniformFields = `
    mat4 matrix;
    vec4 camera;
    vec4 view;
    float radius;
    float blur;
    float opacity;
    float stroke_width;
    float stroke_opacity;
    float translate_anchor;
    float pitch_alignment;
    float pitch_scale;
    vec4 color;
    vec4 stroke_color;
    vec2 translate;
    vec4 interpolation0;
    vec4 interpolation1;
    vec4 interpolation2;
`;
const varyings: PointShader['varyings'] = [['vec2', 'extrude'], ['vec4', 'shape'], ['vec4', 'paint'], ['vec4', 'color'], ['vec4', 'stroke_color']];

// The built-in circle shader (shaders/circle.vertex.glsl and circle.fragment.glsl). The position encodes the tile
// coordinate of the point times two, plus a bit per axis that says on which side of the point the corner is.
const projection = `
    radius = max(radius, 0.0);
    stroke_width = max(stroke_width, 0.0);
    vec2 encoded = vec2(POSITION);
    vec2 center = floor(encoded * 0.5);
    vec2 extrude = (encoded - 2.0 * center) * 2.0 - 1.0;
    if (translate_anchor > 0.5) {
        float c = cos(-u.view.x), s = sin(-u.view.x);
        translate = vec2(c * translate.x - s * translate.y, s * translate.x + c * translate.y);
    }
    center += translate * u.camera.z;
    vec4 projected = u.matrix * vec4(center, 0.0, 1.0);
    float size = radius + stroke_width;
    vec4 position;
    if (pitch_alignment < 0.5) {
        vec2 offset = extrude * size * u.camera.z;
        if (pitch_scale > 0.5) offset *= projected.w / u.camera.w;
        position = u.matrix * vec4(center + offset, 0.0, 1.0);
    } else {
        float scale = pitch_scale < 0.5 ? u.camera.w : projected.w;
        position = projected + vec4(extrude * size * u.camera.xy * scale, 0.0, 0.0);
    }
    OUT_extrude = extrude;
    OUT_shape = vec4(radius, stroke_width, size > 0.0 ? 1.0 / max(u.view.y, 1.0) / size : 0.0, blur);
    OUT_paint = vec4(opacity, stroke_opacity, 0.0, 0.0);
    OUT_color = color;
    OUT_stroke_color = stroke_color;
`;

const coverage = `
    float radius = IN_shape.x;
    float stroke = IN_shape.y;
    float antialiased_blur = -max(IN_shape.w, IN_shape.z);
    float extrude_length = length(IN_extrude);
    float opacity_t = smoothstep(0.0, antialiased_blur, extrude_length - 1.0);
    float color_t = stroke < 0.01 ? 0.0 : smoothstep(antialiased_blur, 0.0, extrude_length - radius / (radius + stroke));
    vec4 result = opacity_t * mix(IN_color * IN_paint.x, IN_stroke_color * IN_paint.y, color_t);
`;

process.stdout.write(shaderHeader({ name: 'circle', properties, uniformFields, varyings, projection, coverage }));
