#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>

namespace mbgl {
namespace shaders {


template <>
struct ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LineShader";
    static const std::array<AttributeInfo, 8> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr const char* vertex = R"(
// Include common functions
const LINE_NORMAL_SCALE: f32 = 1.0 / (127.0 / 2.0);

fn unpack_float(packedValue: f32) -> vec2<f32> {
    let packedIntValue = i32(packedValue);
    let v0 = packedIntValue / 256;
    return vec2<f32>(f32(v0), f32(packedIntValue - v0 * 256));
}

fn decode_color(encodedColor: vec2<f32>) -> vec4<f32> {
    return vec4<f32>(
        unpack_float(encodedColor[0]) / 255.0,
        unpack_float(encodedColor[1]) / 255.0
    );
}

fn unpack_mix_float(packedValue: vec2<f32>, t: f32) -> f32 {
    return mix(packedValue[0], packedValue[1], t);
}

fn unpack_mix_color(packedColors: vec4<f32>, t: f32) -> vec4<f32> {
    let minColor = decode_color(vec2<f32>(packedColors[0], packedColors[1]));
    let maxColor = decode_color(vec2<f32>(packedColors[2], packedColors[3]));
    return mix(minColor, maxColor, t);
}

struct VertexInput {
    @location(4) pos_normal: vec2<i32>,  // packed position and normal
    @location(5) data: vec4<u32>,        // extrude, direction, linesofar
    @location(6) color: vec4<f32>,
    @location(7) blur: vec2<f32>,
    @location(8) opacity: vec2<f32>,
    @location(9) gapwidth: vec2<f32>,
    @location(10) offset: vec2<f32>,
    @location(11) width: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_width2: vec2<f32>,
    @location(1) v_normal: vec2<f32>,
    @location(2) v_gamma_scale: f32,
    @location(3) v_color: vec4<f32>,
    @location(4) v_blur: f32,
    @location(5) v_opacity: f32,
}

struct LineDrawableUBO {
    matrix: mat4x4<f32>,
    ratio: f32,
    color_t: f32,
    blur_t: f32,
    opacity_t: f32,
    gapwidth_t: f32,
    offset_t: f32,
    width_t: f32,
    pad1: f32,
}

struct LineEvaluatedPropsUBO {
    color: vec4<f32>,
    blur: f32,
    opacity: f32,
    gapwidth: f32,
    offset: f32,
    width: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
}

struct GlobalPaintParamsUBO {
    units_to_pixels: vec2<f32>,
    // other fields...
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<LineDrawableUBO>;
@group(0) @binding(4) var<uniform> props: LineEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[globalIndex.value];

    // Constants
    let ANTIALIASING = 1.0 / 2.0;  // Would use DEVICE_PIXEL_RATIO but simplified for WebGPU

    // Unpack vertex data
    let a_extrude = vec2<f32>(f32(in.data.x), f32(in.data.y)) - 128.0;
    let a_direction = f32(in.data.z % 4u) - 1.0;
    let v_linesofar = (f32(in.data.z / 4u) + f32(in.data.w) * 64.0) * 2.0 / 32767.0; // MAX_LINE_DISTANCE

    // Unpack position and normal
    let pos = floor(vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y)) * 0.5);
    let normal = vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y)) - 2.0 * pos;
    let v_normal = vec2<f32>(normal.x, normal.y * 2.0 - 1.0);

    // Unpack attributes using helper functions
    let color = unpack_mix_color(in.color, drawable.color_t);
    let blur = unpack_mix_float(in.blur, drawable.blur_t);
    let opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
    let gapwidth = unpack_mix_float(in.gapwidth, drawable.gapwidth_t) / 2.0;
    let offset = unpack_mix_float(in.offset, drawable.offset_t) * -1.0;
    let width = unpack_mix_float(in.width, drawable.width_t);

    let halfwidth = width / 2.0;
    let inset = gapwidth + select(0.0, ANTIALIASING, gapwidth > 0.0);
    let outset = gapwidth + halfwidth * select(1.0, 2.0, gapwidth > 0.0) + select(0.0, ANTIALIASING, halfwidth != 0.0);

    // Scale the extrusion vector down to a normal and then up by the line width of this vertex
    let dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line
    let u = 0.5 * a_direction;
    let t = 1.0 - abs(u);
    let offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * mat2x2<f32>(t, -u, u, t);

    let projected_extrude = drawable.matrix * vec4<f32>(dist / drawable.ratio, 0.0, 0.0);
    let position = drawable.matrix * vec4<f32>(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;

    // Calculate gamma scale for antialiasing
    let extrude_length_without_perspective = length(dist);
    let extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    // Convert to NDC coordinates and flip Y for WebGPU
    // WebGPU uses Z range [0, 1] instead of [-1, 1]
    let ndc_z = (position.z / position.w) * 0.5 + 0.5;
    out.position = vec4<f32>(
        position.x / position.w,
        -position.y / position.w,  // Flip Y after perspective divide
        ndc_z,
        1.0
    );
    out.v_width2 = vec2<f32>(outset, inset);
    out.v_normal = v_normal;
    out.v_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;
    out.v_color = color;
    out.v_blur = blur;
    out.v_opacity = opacity;

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) v_width2: vec2<f32>,
    @location(1) v_normal: vec2<f32>,
    @location(2) v_gamma_scale: f32,
    @location(3) v_color: vec4<f32>,
    @location(4) v_blur: f32,
    @location(5) v_opacity: f32,
}

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Calculate the distance of the pixel from the line in pixels
    let dist = length(in.v_normal) * in.v_width2.x;

    // Calculate the antialiasing fade factor
    let blur2 = (in.v_blur + 1.0 / 2.0) * in.v_gamma_scale;  // Would use DEVICE_PIXEL_RATIO
    let alpha = clamp(min(dist - (in.v_width2.y - blur2), in.v_width2.x - dist) / blur2, 0.0, 1.0);

    return in.v_color * (alpha * in.v_opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LineGradientShader";
    static const std::array<AttributeInfo, 7> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(4) pos_normal: vec2<i32>,
    @location(5) data: vec4<u32>,
    @location(6) blur: vec2<f32>,
    @location(7) opacity: vec2<f32>,
    @location(8) gapwidth: vec2<f32>,
    @location(9) offset: vec2<f32>,
    @location(10) width: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_width2: vec2<f32>,
    @location(1) v_normal: vec2<f32>,
    @location(2) v_gamma_scale: f32,
    @location(3) v_lineprogress: f32,
    @location(4) v_blur: f32,
    @location(5) v_opacity: f32,
}

struct LineGradientDrawableUBO {
    matrix: mat4x4<f32>,
    ratio: f32,
    blur_t: f32,
    opacity_t: f32,
    gapwidth_t: f32,
    offset_t: f32,
    width_t: f32,
    pad1: f32,
    pad2: f32,
}

struct LineEvaluatedPropsUBO {
    color: vec4<f32>,
    blur: f32,
    opacity: f32,
    gapwidth: f32,
    offset: f32,
    width: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
}

struct GlobalPaintParamsUBO {
    units_to_pixels: vec2<f32>,
    // other fields...
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<LineGradientDrawableUBO>;
@group(0) @binding(4) var<uniform> props: LineEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[globalIndex.value];

    // Constants
    let ANTIALIASING = 1.0 / 2.0;
    let MAX_LINE_DISTANCE = 32767.0;

    // Unpack vertex data
    let a_extrude = vec2<f32>(f32(in.data.x), f32(in.data.y)) - 128.0;
    let a_direction = f32(in.data.z % 4u) - 1.0;
    let v_lineprogress = (f32(in.data.z / 4u) + f32(in.data.w) * 64.0) * 2.0 / MAX_LINE_DISTANCE;

    // Unpack position and normal
    let pos = floor(vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y)) * 0.5);
    let normal = vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y)) - 2.0 * pos;
    let v_normal = vec2<f32>(normal.x, normal.y * 2.0 - 1.0);

    // Unpack attributes using helper functions
    let blur = unpack_mix_float(in.blur, drawable.blur_t);
    let opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
    let gapwidth = unpack_mix_float(in.gapwidth, drawable.gapwidth_t) / 2.0;
    let offset = unpack_mix_float(in.offset, drawable.offset_t) * -1.0;
    let width = unpack_mix_float(in.width, drawable.width_t);

    let halfwidth = width / 2.0;
    let inset = gapwidth + select(0.0, ANTIALIASING, gapwidth > 0.0);
    let outset = gapwidth + halfwidth * select(1.0, 2.0, gapwidth > 0.0) + select(0.0, ANTIALIASING, halfwidth != 0.0);

    // Scale the extrusion vector down to a normal and then up by the line width
    let dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset
    let u = 0.5 * a_direction;
    let t = 1.0 - abs(u);
    let offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * mat2x2<f32>(t, -u, u, t);

    let projected_extrude = drawable.matrix * vec4<f32>(dist / drawable.ratio, 0.0, 0.0);
    let position = drawable.matrix * vec4<f32>(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;

    // Calculate gamma scale
    let extrude_length_without_perspective = length(dist);
    let extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    out.position = position;
    out.v_width2 = vec2<f32>(outset, inset);
    out.v_normal = v_normal;
    out.v_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;
    out.v_lineprogress = v_lineprogress;
    out.v_blur = blur;
    out.v_opacity = opacity;

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) v_width2: vec2<f32>,
    @location(1) v_normal: vec2<f32>,
    @location(2) v_gamma_scale: f32,
    @location(3) v_lineprogress: f32,
    @location(4) v_blur: f32,
    @location(5) v_opacity: f32,
}

@group(1) @binding(0) var gradient_sampler: sampler;
@group(1) @binding(1) var gradient_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Calculate the distance of the pixel from the line
    let dist = length(in.v_normal) * in.v_width2.x;

    // Calculate the antialiasing fade factor
    let blur2 = (in.v_blur + 1.0 / 2.0) * in.v_gamma_scale;
    let alpha = clamp(min(dist - (in.v_width2.y - blur2), in.v_width2.x - dist) / blur2, 0.0, 1.0);

    // Sample gradient texture
    let color = textureSample(gradient_texture, gradient_sampler, vec2<f32>(in.v_lineprogress, 0.5));

    return color * (alpha * in.v_opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LinePatternShader";
    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(4) pos_normal: vec2<i32>,
    @location(5) data: vec4<u32>,
    @location(6) blur: vec2<f32>,
    @location(7) opacity: vec2<f32>,
    @location(8) gapwidth: vec2<f32>,
    @location(9) offset: vec2<f32>,
    @location(10) width: vec2<f32>,
    @location(11) pattern_from: vec4<u32>,
    @location(12) pattern_to: vec4<u32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_width2: vec2<f32>,
    @location(1) v_normal: vec2<f32>,
    @location(2) v_gamma_scale: f32,
    @location(3) v_linesofar: f32,
    @location(4) v_blur: f32,
    @location(5) v_opacity: f32,
    @location(6) v_pattern_from: vec4<f32>,
    @location(7) v_pattern_to: vec4<f32>,
}

struct LinePatternDrawableUBO {
    matrix: mat4x4<f32>,
    ratio: f32,
    blur_t: f32,
    opacity_t: f32,
    gapwidth_t: f32,
    offset_t: f32,
    width_t: f32,
    pattern_from_t: f32,
    pattern_to_t: f32,
}

struct LinePatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    scale: vec4<f32>,
    texsize: vec2<f32>,
    fade: f32,
    pad2: f32,
}

struct LineEvaluatedPropsUBO {
    color: vec4<f32>,
    blur: f32,
    opacity: f32,
    gapwidth: f32,
    offset: f32,
    width: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
}

struct GlobalPaintParamsUBO {
    units_to_pixels: vec2<f32>,
    pixel_ratio: f32,
    // other fields...
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<LinePatternDrawableUBO>;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<LinePatternTilePropsUBO>;
@group(0) @binding(4) var<uniform> props: LineEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let index = globalIndex.value;
    let drawable = drawableVector[index];
    let tileProps = tilePropsVector[index];

    // Constants
    let ANTIALIASING = 1.0 / 2.0;
    let MAX_LINE_DISTANCE = 32767.0;

    // Unpack vertex data
    let a_extrude = vec2<f32>(f32(in.data.x), f32(in.data.y)) - 128.0;
    let a_direction = f32(in.data.z % 4u) - 1.0;
    let v_linesofar = (f32(in.data.z / 4u) + f32(in.data.w) * 64.0) * 2.0 / MAX_LINE_DISTANCE;

    // Unpack position and normal
    let pos = floor(vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y)) * 0.5);
    let normal = vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y)) - 2.0 * pos;
    let v_normal = vec2<f32>(normal.x, normal.y * 2.0 - 1.0);

    // Unpack attributes using helper functions
    let blur = unpack_mix_float(in.blur, drawable.blur_t);
    let opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
    let gapwidth = unpack_mix_float(in.gapwidth, drawable.gapwidth_t) / 2.0;
    let offset = unpack_mix_float(in.offset, drawable.offset_t) * -1.0;
    let width = unpack_mix_float(in.width, drawable.width_t);
    let pattern_from = vec4<f32>(in.pattern_from);
    let pattern_to = vec4<f32>(in.pattern_to);

    let halfwidth = width / 2.0;
    let inset = gapwidth + select(0.0, ANTIALIASING, gapwidth > 0.0);
    let outset = gapwidth + halfwidth * select(1.0, 2.0, gapwidth > 0.0) + select(0.0, ANTIALIASING, halfwidth != 0.0);

    // Scale the extrusion vector
    let dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset
    let u = 0.5 * a_direction;
    let t = 1.0 - abs(u);
    let offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * mat2x2<f32>(t, -u, u, t);

    let projected_extrude = drawable.matrix * vec4<f32>(dist / drawable.ratio, 0.0, 0.0);
    let position = drawable.matrix * vec4<f32>(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;

    // Calculate gamma scale
    let extrude_length_without_perspective = length(dist);
    let extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    out.position = position;
    out.v_width2 = vec2<f32>(outset, inset);
    out.v_normal = v_normal;
    out.v_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;
    out.v_linesofar = v_linesofar;
    out.v_blur = blur;
    out.v_opacity = opacity;
    out.v_pattern_from = pattern_from;
    out.v_pattern_to = pattern_to;

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) v_width2: vec2<f32>,
    @location(1) v_normal: vec2<f32>,
    @location(2) v_gamma_scale: f32,
    @location(3) v_linesofar: f32,
    @location(4) v_blur: f32,
    @location(5) v_opacity: f32,
    @location(6) v_pattern_from: vec4<f32>,
    @location(7) v_pattern_to: vec4<f32>,
}

struct LinePatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    scale: vec4<f32>,
    texsize: vec2<f32>,
    fade: f32,
    pad2: f32,
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

@group(0) @binding(3) var<storage, read> tilePropsVector: array<LinePatternTilePropsUBO>;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(1) @binding(0) var pattern_sampler: sampler;
@group(1) @binding(1) var pattern_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value];

    // Calculate the distance of the pixel from the line
    let dist = length(in.v_normal) * in.v_width2.x;

    // Calculate the antialiasing fade factor
    let blur2 = (in.v_blur + 1.0 / 2.0) * in.v_gamma_scale;
    let alpha = clamp(min(dist - (in.v_width2.y - blur2), in.v_width2.x - dist) / blur2, 0.0, 1.0);

    // Pattern sampling
    let pattern_tl_a = in.v_pattern_from.xy;
    let pattern_br_a = in.v_pattern_from.zw;
    let pattern_tl_b = in.v_pattern_to.xy;
    let pattern_br_b = in.v_pattern_to.zw;

    let imagecoord = gl_mod(vec2<f32>(in.v_linesofar, 0.5), vec2<f32>(1.0));
    let pos_a = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord);
    let pos_b = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord);

    let color_a = textureSample(pattern_texture, pattern_sampler, pos_a);
    let color_b = textureSample(pattern_texture, pattern_sampler, pos_b);

    let pattern_color = mix(color_a, color_b, tileProps.fade);

    return pattern_color * (alpha * in.v_opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LineSDFShader";
    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(4) pos_normal: vec2<i32>,
    @location(5) data: vec4<u32>,
    @location(6) color: vec4<f32>,
    @location(7) blur: vec2<f32>,
    @location(8) opacity: vec2<f32>,
    @location(9) gapwidth: vec2<f32>,
    @location(10) offset: vec2<f32>,
    @location(11) width: vec2<f32>,
    @location(12) floorwidth: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_width2: vec2<f32>,
    @location(1) v_normal: vec2<f32>,
    @location(2) v_gamma_scale: f32,
    @location(3) v_tex_a: vec2<f32>,
    @location(4) v_tex_b: vec2<f32>,
    @location(5) v_color: vec4<f32>,
    @location(6) v_blur: f32,
    @location(7) v_opacity: f32,
    @location(8) v_floorwidth: f32,
}

struct LineSDFDrawableUBO {
    matrix: mat4x4<f32>,
    patternscale_a: vec2<f32>,
    patternscale_b: vec2<f32>,
    tex_y_a: f32,
    tex_y_b: f32,
    ratio: f32,
    color_t: f32,
    blur_t: f32,
    opacity_t: f32,
    gapwidth_t: f32,
    offset_t: f32,
    width_t: f32,
    floorwidth_t: f32,
    pad1: f32,
    pad2: f32,
}

struct LineSDFTilePropsUBO {
    sdfgamma: f32,
    mix: f32,
    pad1: f32,
    pad2: f32,
}

struct LineEvaluatedPropsUBO {
    color: vec4<f32>,
    blur: f32,
    opacity: f32,
    gapwidth: f32,
    offset: f32,
    width: f32,
    floorwidth: f32,
    pad1: f32,
    pad2: f32,
}

struct GlobalPaintParamsUBO {
    units_to_pixels: vec2<f32>,
    // other fields...
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<LineSDFDrawableUBO>;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<LineSDFTilePropsUBO>;
@group(0) @binding(4) var<uniform> props: LineEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let index = globalIndex.value;
    let drawable = drawableVector[index];
    let tileProps = tilePropsVector[index];

    // Constants
    let ANTIALIASING = 1.0 / 2.0;
    let MAX_LINE_DISTANCE = 32767.0;

    // Unpack vertex data
    let a_extrude = vec2<f32>(f32(in.data.x), f32(in.data.y)) - 128.0;
    let a_direction = f32(in.data.z % 4u) - 1.0;
    let v_linesofar = (f32(in.data.z / 4u) + f32(in.data.w) * 64.0) * 2.0 / MAX_LINE_DISTANCE;

    // Unpack position and normal
    let pos = floor(vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y)) * 0.5);
    let normal = vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y)) - 2.0 * pos;
    let v_normal = vec2<f32>(normal.x, normal.y * 2.0 - 1.0);

    // Unpack attributes using helper functions
    let color = unpack_mix_color(in.color, drawable.color_t);
    let blur = unpack_mix_float(in.blur, drawable.blur_t);
    let opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
    let gapwidth = unpack_mix_float(in.gapwidth, drawable.gapwidth_t) / 2.0;
    let offset = unpack_mix_float(in.offset, drawable.offset_t) * -1.0;
    let width = unpack_mix_float(in.width, drawable.width_t);
    let floorwidth = unpack_mix_float(in.floorwidth, drawable.floorwidth_t);

    let halfwidth = width / 2.0;
    let inset = gapwidth + select(0.0, ANTIALIASING, gapwidth > 0.0);
    let outset = gapwidth + halfwidth * select(1.0, 2.0, gapwidth > 0.0) + select(0.0, ANTIALIASING, halfwidth != 0.0);

    // Scale the extrusion vector
    let dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset
    let u = 0.5 * a_direction;
    let t = 1.0 - abs(u);
    let offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * mat2x2<f32>(t, -u, u, t);

    let projected_extrude = drawable.matrix * vec4<f32>(dist / drawable.ratio, 0.0, 0.0);
    let position = drawable.matrix * vec4<f32>(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;

    // Calculate gamma scale
    let extrude_length_without_perspective = length(dist);
    let extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    // Calculate texture coordinates
    let tex_a = vec2<f32>(
        v_linesofar * drawable.patternscale_a.x / floorwidth,
        normal.y * drawable.patternscale_a.y + drawable.tex_y_a
    );
    let tex_b = vec2<f32>(
        v_linesofar * drawable.patternscale_b.x / floorwidth,
        normal.y * drawable.patternscale_b.y + drawable.tex_y_b
    );

    out.position = position;
    out.v_width2 = vec2<f32>(outset, inset);
    out.v_normal = v_normal;
    out.v_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;
    out.v_tex_a = tex_a;
    out.v_tex_b = tex_b;
    out.v_color = color;
    out.v_blur = blur;
    out.v_opacity = opacity;
    out.v_floorwidth = floorwidth;

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) v_width2: vec2<f32>,
    @location(1) v_normal: vec2<f32>,
    @location(2) v_gamma_scale: f32,
    @location(3) v_tex_a: vec2<f32>,
    @location(4) v_tex_b: vec2<f32>,
    @location(5) v_color: vec4<f32>,
    @location(6) v_blur: f32,
    @location(7) v_opacity: f32,
    @location(8) v_floorwidth: f32,
}

struct LineSDFTilePropsUBO {
    sdfgamma: f32,
    mix: f32,
    pad1: f32,
    pad2: f32,
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

@group(0) @binding(3) var<storage, read> tilePropsVector: array<LineSDFTilePropsUBO>;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(1) @binding(0) var sdf_sampler: sampler;
@group(1) @binding(1) var sdf_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value];

    // Calculate the distance of the pixel from the line
    let dist = length(in.v_normal) * in.v_width2.x;

    // Calculate the antialiasing fade factor
    let blur2 = (in.v_blur + 1.0 / 2.0) * in.v_gamma_scale;
    let alpha = clamp(min(dist - (in.v_width2.y - blur2), in.v_width2.x - dist) / blur2, 0.0, 1.0);

    // Sample SDF texture
    let dist_a = textureSample(sdf_texture, sdf_sampler, in.v_tex_a).a;
    let dist_b = textureSample(sdf_texture, sdf_sampler, in.v_tex_b).a;
    let sdfdist = mix(dist_a, dist_b, tileProps.mix);

    // Calculate SDF alpha
    let sdf_alpha = smoothstep(0.5 - tileProps.sdfgamma / in.v_floorwidth, 0.5 + tileProps.sdfgamma / in.v_floorwidth, sdfdist);

    return in.v_color * (alpha * in.v_opacity * sdf_alpha);
}
)";
};

} // namespace shaders
} // namespace mbgl
