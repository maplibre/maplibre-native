// Generates the checked-in backend sources from one polygon/projection algorithm.
// Print to stdout, or use --check to verify the checked-in result.
import {readFileSync} from 'node:fs';

const properties = [
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
const varyings = [['vec2', 'local'], ['vec4', 'shape'], ['vec4', 'paint'], ['vec4', 'color'], ['vec4', 'stroke_color']];
const macro = name => `MLN_PLUGIN_PROPERTY_NGON_${name.toUpperCase()}_IS_UNIFORM`;
const metal = s => s.replaceAll('mat4', 'float4x4').replace(/vec([234])/g, 'float$1')
  .replaceAll('atan(', 'atan2(').replaceAll('mod(', 'fmod(');

function attributes(backend) {
  let result = backend === 'metal' ? '    short2 a_position [[attribute(0)]];\n'
    : backend === 'vulkan' ? 'layout(location = 0) in ivec2 a_position;\n' : 'in vec2 a_position;\n';
  for (const [name, type, min, max] of properties) {
    result += `#if !${macro(name)}\n`;
    const inputs = min === max ? [[name, type === 'vec2' ? 'vec4' : 'vec2', min]]
      : [[`${name}_min`, type, min], [`${name}_max`, type, max]];
    for (const [n, t, id] of inputs) {
      result += backend === 'metal' ? `    ${metal(t)} a_${n} [[attribute(${id})]];\n`
        : `${backend === 'vulkan' ? `layout(location = ${id}) ` : ''}in ${t} a_${n};\n`;
    }
    result += '#endif\n';
  }
  return result;
}

function evaluate(backend) {
  let result = '';
  properties.forEach(([name, type, min, max], i) => {
    const a = backend === 'metal' ? 'in.' : '';
    const factor = `u.interpolation${Math.floor(i / 4)}.${'xyzw'[i % 4]}`;
    const low = min !== max ? `${a}a_${name}_min` : `${a}a_${name}.${type === 'vec2' ? 'xy' : 'x'}`;
    const high = min !== max ? `${a}a_${name}_max` : `${a}a_${name}.${type === 'vec2' ? 'zw' : 'y'}`;
    result += `    ${type === 'enum' ? 'float' : type} ${name} = u.${name};\n#if !${macro(name)}\n`;
    result += `    ${name} = ${type === 'enum' ? `${factor} < 1.0 ? ${low} : ${high}` : `mix(${low}, ${high}, ${factor})`};\n#endif\n`;
  });
  return result;
}

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

let output = '// Generated by scripts/generate-shaders.mjs. Do not edit by hand.\n// clang-format off\n#pragma once\n\n';
const emit = (name, source) => { output += `inline constexpr char ${name}[] = R"SHADER(\n${source})SHADER";\n\n`; };
for (const backend of ['opengl', 'vulkan', 'metal']) {
  const vertexBody = evaluate(backend) + projection.replace('POSITION', backend === 'metal' ? 'in.a_position' : 'a_position')
    .replaceAll('OUT_', backend === 'metal' ? 'out.' : 'v_');
  if (backend === 'metal') {
    const source = `struct alignas(16) NgonDrawableUBO {${metal(uniformFields)}};
struct NgonVertex {\n${attributes(backend)}};
struct NgonVaryings {
    float4 position [[position]];
${varyings.map(([t, n]) => `    ${metal(t)} ${n};`).join('\n')}
};
vertex NgonVaryings ngonVertex(NgonVertex in [[stage_in]],
    constant NgonDrawableUBO& u [[buffer(MLN_PLUGIN_UNIFORM_0_BINDING)]]) {
    NgonVaryings out;
${metal(vertexBody)}    out.position = position;
    return out;
}
fragment half4 ngonFragment(NgonVaryings in [[stage_in]]) {
${metal(coverage.replaceAll('IN_', 'in.'))}    return half4(result);
}
`;
    emit('metalSource', source);
  } else {
    const io = direction => varyings.map(([t, n], i) => `${backend === 'vulkan' ? `layout(location = ${i}) ` : ''}${direction} ${t} v_${n};\n`).join('');
    const layout = backend === 'vulkan'
      ? 'layout(std140, set = DRAWABLE_UBO_SET_INDEX, binding = MLN_PLUGIN_UNIFORM_0_BINDING)'
      : 'layout(std140)';
    emit(backend + 'Vertex', `${attributes(backend)}\n${layout} uniform NgonDrawableUBO {${uniformFields}} u;\n${io('out')}
void main() {
${vertexBody}    gl_Position = position;
${backend === 'vulkan' ? '    applySurfaceTransform();\n' : ''}}
`);
    emit(backend + 'Fragment', `${io('in')}${backend === 'vulkan' ? 'layout(location = 0) out vec4 fragColor;\n' : ''}
void main() {
${coverage.replaceAll('IN_', 'v_')}    fragColor = result;
}
`);
  }
}

output += 'const mln_plugin_shader_attribute_v1 shaderAttributes[] = {\n    {sizeof(mln_plugin_shader_attribute_v1), 0, 0, str("a_position"), MLN_PLUGIN_VERTEX_INT16_X2},\n';
for (const [name, type, min, max] of properties) {
  for (const [n, id] of min === max ? [[name, min]] : [[name + '_min', min], [name + '_max', max]]) {
    const size = type === 'vec4' || type === 'vec2' ? 4 : 2;
    output += `    {sizeof(mln_plugin_shader_attribute_v1), ${id}, ${id}, str("a_${n}"), MLN_PLUGIN_VERTEX_FLOAT_X${size}},\n`;
  }
}
output += '};\n\nconst mln_plugin_shader_property_binding_v1 propertyBindings[] = {\n';
properties.forEach(([name, type, min, max], i) => {
  const encoding = {float:'FLOAT',vec2:'FLOAT2',vec4:'COLOR',enum:'ENUM_FLOAT'}[type];
  output += `    {sizeof(mln_plugin_shader_property_binding_v1), str("ngon-${name.replaceAll('_','-')}"), MLN_PLUGIN_PROPERTY_ENCODING_${encoding}, 0, offsetof(DrawableUBO, ${name}), ${min}, ${max}, 0, offsetof(DrawableUBO, interpolation) + ${i} * sizeof(float)},\n`;
});
output += '};\n';
if (process.argv.includes('--check')) {
  const existing = readFileSync(new URL('../shared/include/ngon_shader_sources.hpp', import.meta.url), 'utf8');
  if (existing !== output) throw new Error('Regenerate ngon_shader_sources.hpp');
} else process.stdout.write(output);
