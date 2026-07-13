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
struct VertexInput {
    @location(4) pos_normal: vec2<i32>,  // packed position and normal
    @location(5) data: vec4<u32>,        // extrude, direction, linesofar
    @location(6) color: vec4<f32>,
    @location(7) blur: vec2<f32>,
    @location(8) opacity: vec2<f32>,
    @location(9) gapwidth: vec2<f32>,
    @location(10) offset: vec2<f32>,
    @location(11) width: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_width2: vec2<f32>,
    @location(1) v_normal: vec2<f32>,
    @location(2) v_gamma_scale: f32,
    @location(3) v_color: vec4<f32>,
    @location(4) v_blur: f32,
    @location(5) v_opacity: f32,
};

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
};

struct LineEvaluatedPropsUBO {
    color: vec4<f32>,
    blur: f32,
    opacity: f32,
    gapwidth: f32,
    offset: f32,
    width: f32,
    floorwidth: f32,
    expressionMask: u32,
    pad1: f32,
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct LineDrawableEntry {
    data: LineDrawableUBO,
    pad0: vec4<f32>,
    pad1: vec4<f32>,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<LineDrawableEntry>;
@group(0) @binding(4) var<uniform> props: LineEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[globalIndex.value].data;
    let ratio = max(drawable.ratio, 1e-6);
    let pixel_ratio = paintParams.pixel_ratio;
    let antialiasing = (1.0 / pixel_ratio) * 0.5;

    // Unpack vertex data
    let a_extrude = vec2<f32>(f32(in.data.x), f32(in.data.y)) - vec2<f32>(128.0, 128.0);
    let a_direction = f32(in.data.z & 3u) - 1.0;
    let raw_pos = vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y));
    let pos = floor(raw_pos * 0.5);
    var normal = raw_pos - pos * 2.0;
    normal.y = normal.y * 2.0 - 1.0;

    var color: vec4<f32>;
    color = props.color;
#ifndef HAS_UNIFORM_u_color
    color = unpack_mix_color(in.color, drawable.color_t);
#endif

    var blur: f32;
    blur = props.blur;
#ifndef HAS_UNIFORM_u_blur
    blur = unpack_mix_float(in.blur, drawable.blur_t);
#endif

    var opacity: f32;
    opacity = props.opacity;
#ifndef HAS_UNIFORM_u_opacity
    opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
#endif

    var gapwidth: f32;
    gapwidth = props.gapwidth;
#ifndef HAS_UNIFORM_u_gapwidth
    gapwidth = unpack_mix_float(in.gapwidth, drawable.gapwidth_t);
#endif
    gapwidth = gapwidth / 2.0;

    var offset: f32;
    offset = props.offset;
#ifndef HAS_UNIFORM_u_offset
    offset = unpack_mix_float(in.offset, drawable.offset_t);
#endif
    offset = -offset;

    var width: f32;
    width = props.width;
#ifndef HAS_UNIFORM_u_width
    width = unpack_mix_float(in.width, drawable.width_t);
#endif

    let halfwidth = width * 0.5;
    let inset = gapwidth + select(0.0, antialiasing, gapwidth > 0.0);
    let outset = gapwidth + halfwidth * select(1.0, 2.0, gapwidth > 0.0) +
                 select(0.0, antialiasing, halfwidth != 0.0);

    // Scale the extrusion vector down to a normal and then up by the line width of this vertex
    let dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line
    let u = 0.5 * a_direction;
    let t = 1.0 - abs(u);
    let offset2 = offset * a_extrude * LINE_NORMAL_SCALE * normal.y * mat2x2<f32>(t, -u, u, t);

    let projected_extrude = drawable.matrix * vec4<f32>(dist / ratio, 0.0, 0.0);
    let base = drawable.matrix * vec4<f32>(pos + offset2 / ratio, 0.0, 1.0);
    let clip = base + projected_extrude;

    let inv_w = 1.0 / clip.w;

    out.position = clip;

    let extrude_length_without_perspective = length(dist);
    let extrude_length_with_perspective =
        length((projected_extrude.xy / clip.w) * paintParams.units_to_pixels);
    let gamma_denom = max(extrude_length_with_perspective, 1e-6);

    out.v_width2 = vec2<f32>(outset, inset);
    out.v_normal = normal;
    out.v_gamma_scale = extrude_length_without_perspective / gamma_denom;
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
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Calculate the distance of the pixel from the line in pixels
    let dist = length(in.v_normal) * in.v_width2.x;

    // Calculate the antialiasing fade factor
    let antialiasing = 0.5 / paintParams.pixel_ratio;
    let blur2 = (in.v_blur + (1.0 / paintParams.pixel_ratio)) * in.v_gamma_scale;
    let denom = max(blur2, 1e-6);
    let alpha = clamp(min(dist - (in.v_width2.y - blur2), in.v_width2.x - dist) / denom, 0.0, 1.0);

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
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_width2: vec2<f32>,
    @location(1) v_normal: vec2<f32>,
    @location(2) v_gamma_scale: f32,
    @location(3) v_lineprogress: f32,
    @location(4) v_blur: f32,
    @location(5) v_opacity: f32,
};

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
};

const LINE_EXPRESSION_COLOR: u32 = 1u << 0u;
const LINE_EXPRESSION_OPACITY: u32 = 1u << 1u;
const LINE_EXPRESSION_BLUR: u32 = 1u << 2u;
const LINE_EXPRESSION_WIDTH: u32 = 1u << 3u;
const LINE_EXPRESSION_GAPWIDTH: u32 = 1u << 4u;
const LINE_EXPRESSION_FLOORWIDTH: u32 = 1u << 5u;
const LINE_EXPRESSION_OFFSET: u32 = 1u << 6u;

struct LineEvaluatedPropsUBO {
    color: vec4<f32>,
    blur: f32,
    opacity: f32,
    gapwidth: f32,
    offset: f32,
    width: f32,
    floorwidth: f32,
    expressionMask: u32,
    pad1: f32,
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct LineGradientDrawableEntry {
    data: LineGradientDrawableUBO,
    pad0: vec4<f32>,
    pad1: vec4<f32>,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<LineGradientDrawableEntry>;
@group(0) @binding(4) var<uniform> props: LineEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[globalIndex.value].data;
    let mask = props.expressionMask;

    // Constants
    let pixel_ratio = max(paintParams.pixel_ratio, 1e-6);
    let antialiasing = 0.5 / pixel_ratio;
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
    var blur = unpack_mix_float(in.blur, drawable.blur_t);
    if ((mask & LINE_EXPRESSION_BLUR) != 0u) {
        blur = props.blur;
    }

    var opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
    if ((mask & LINE_EXPRESSION_OPACITY) != 0u) {
        opacity = props.opacity;
    }

    var gapwidth = unpack_mix_float(in.gapwidth, drawable.gapwidth_t);
    if ((mask & LINE_EXPRESSION_GAPWIDTH) != 0u) {
        gapwidth = props.gapwidth;
    }
    gapwidth = gapwidth / 2.0;

    var offset = unpack_mix_float(in.offset, drawable.offset_t);
    if ((mask & LINE_EXPRESSION_OFFSET) != 0u) {
        offset = props.offset;
    }
    offset = -offset;

    var width = unpack_mix_float(in.width, drawable.width_t);
    if ((mask & LINE_EXPRESSION_WIDTH) != 0u) {
        width = props.width;
    }

    let halfwidth = width / 2.0;
    let inset = gapwidth + select(0.0, antialiasing, gapwidth > 0.0);
    let outset = gapwidth + halfwidth * select(1.0, 2.0, gapwidth > 0.0) +
                 select(0.0, antialiasing, halfwidth != 0.0);

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
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(1) @binding(0) var gradient_sampler: sampler;
@group(1) @binding(1) var gradient_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Calculate the distance of the pixel from the line
    let dist = length(in.v_normal) * in.v_width2.x;

    // Calculate the antialiasing fade factor
    let pixel_ratio = max(paintParams.pixel_ratio, 1e-6);
    let blur2 = (in.v_blur + 1.0 / pixel_ratio) * in.v_gamma_scale;
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
};

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
};

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
};

struct LinePatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    scale: vec4<f32>,
    texsize: vec2<f32>,
    fade: f32,
    pad2: f32,
};

struct LineEvaluatedPropsUBO {
    color: vec4<f32>,
    blur: f32,
    opacity: f32,
    gapwidth: f32,
    offset: f32,
    width: f32,
    floorwidth: f32,
    expressionMask: u32,
    pad1: f32,
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct LinePatternDrawableEntry {
    data: LinePatternDrawableUBO,
    pad0: vec4<f32>,
    pad1: vec4<f32>,
};

struct LinePatternTilePropsEntry {
    data: LinePatternTilePropsUBO,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<LinePatternDrawableEntry>;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<LinePatternTilePropsEntry>;
@group(0) @binding(4) var<uniform> props: LineEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let index = globalIndex.value;
    let drawable = drawableVector[index].data;
    let tileProps = tilePropsVector[index].data;

    let ratio = max(drawable.ratio, 1e-6);
    let antialiasing = 0.5 / paintParams.pixel_ratio;
    let lineDistanceScale = 2.0;

    let a_extrude = vec2<f32>(f32(in.data.x), f32(in.data.y)) - 128.0;
    let a_direction = f32(in.data.z & 3u) - 1.0;
    let v_linesofar = (floor(f32(in.data.z) * 0.25) + f32(in.data.w) * 64.0) * lineDistanceScale;

    let raw_pos = vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y));
    let pos = floor(raw_pos * 0.5);
    let normal = raw_pos - 2.0 * pos;
    let v_normal = vec2<f32>(normal.x, normal.y * 2.0 - 1.0);

    var blur = props.blur;
#ifndef HAS_UNIFORM_u_blur
    blur = unpack_mix_float(in.blur, drawable.blur_t);
#endif

    var opacity = props.opacity;
#ifndef HAS_UNIFORM_u_opacity
    opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
#endif

    var gapwidth = props.gapwidth;
#ifndef HAS_UNIFORM_u_gapwidth
    gapwidth = unpack_mix_float(in.gapwidth, drawable.gapwidth_t);
#endif
    gapwidth = gapwidth * 0.5;

    var offset = props.offset;
#ifndef HAS_UNIFORM_u_offset
    offset = unpack_mix_float(in.offset, drawable.offset_t);
#endif
    offset = -offset;

    var width = props.width;
#ifndef HAS_UNIFORM_u_width
    width = unpack_mix_float(in.width, drawable.width_t);
#endif

    var pattern_from = tileProps.pattern_from;
#ifndef HAS_UNIFORM_u_pattern_from
    pattern_from = vec4<f32>(in.pattern_from);
#endif
    var pattern_to = tileProps.pattern_to;
#ifndef HAS_UNIFORM_u_pattern_to
    pattern_to = vec4<f32>(in.pattern_to);
#endif

    let halfwidth = width * 0.5;
    let inset = gapwidth + select(0.0, antialiasing, gapwidth > 0.0);
    let outset = gapwidth + halfwidth * select(1.0, 2.0, gapwidth > 0.0) + select(0.0, antialiasing, halfwidth != 0.0);

    let dist = outset * a_extrude * LINE_NORMAL_SCALE;

    let u = 0.5 * a_direction;
    let t = 1.0 - abs(u);
    let offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * mat2x2<f32>(t, -u, u, t);

    let projected_extrude = drawable.matrix * vec4<f32>(dist / ratio, 0.0, 0.0);
    let position = drawable.matrix * vec4<f32>(pos + offset2 / ratio, 0.0, 1.0) + projected_extrude;

    let extrude_length_without_perspective = length(dist);
    let extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);
    let gamma_scale = extrude_length_without_perspective / max(extrude_length_with_perspective, 1e-6);

    out.position = position;
    out.v_width2 = vec2<f32>(outset, inset);
    out.v_normal = v_normal;
    out.v_gamma_scale = gamma_scale;
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
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

struct LinePatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    scale: vec4<f32>,
    texsize: vec2<f32>,
    fade: f32,
    pad2: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct LinePatternTilePropsEntry {
    data: LinePatternTilePropsUBO,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<LinePatternTilePropsEntry>;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(1) @binding(0) var pattern_sampler: sampler;
@group(1) @binding(1) var pattern_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value].data;

    var pattern_from = in.v_pattern_from;
#ifdef HAS_UNIFORM_u_pattern_from
    pattern_from = tileProps.pattern_from;
#endif
    var pattern_to = in.v_pattern_to;
#ifdef HAS_UNIFORM_u_pattern_to
    pattern_to = tileProps.pattern_to;
#endif

    let pattern_tl_a = pattern_from.xy;
    let pattern_br_a = pattern_from.zw;
    let pattern_tl_b = pattern_to.xy;
    let pattern_br_b = pattern_to.zw;

    let pixelRatio = tileProps.scale.x;
    let tileZoomRatio = max(tileProps.scale.y, 1e-6);
    let fromScale = tileProps.scale.z;
    let toScale = tileProps.scale.w;

    let display_size_a = vec2<f32>((pattern_br_a.x - pattern_tl_a.x) / pixelRatio,
                                   (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    let display_size_b = vec2<f32>((pattern_br_b.x - pattern_tl_b.x) / pixelRatio,
                                   (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    let pattern_size_a = vec2<f32>(display_size_a.x * fromScale / tileZoomRatio, display_size_a.y);
    let pattern_size_b = vec2<f32>(display_size_b.x * toScale / tileZoomRatio, display_size_b.y);

    let dist = length(in.v_normal) * in.v_width2.x;
    let blur = in.v_blur;
    let opacity = in.v_opacity;
    let invPixelRatio = 1.0 / max(paintParams.pixel_ratio, 1e-6);
    let blur2 = (blur + invPixelRatio) * in.v_gamma_scale;
    let alpha = clamp(min(dist - (in.v_width2.y - blur2), in.v_width2.x - dist) / blur2, 0.0, 1.0);

    let patternSizeAX = max(pattern_size_a.x, 1e-6);
    let patternSizeBX = max(pattern_size_b.x, 1e-6);
    let patternSizeAY = max(pattern_size_a.y, 1e-6);
    let patternSizeBY = max(pattern_size_b.y, 1e-6);

    let x_a = glMod(in.v_linesofar / patternSizeAX, 1.0);
    let x_b = glMod(in.v_linesofar / patternSizeBX, 1.0);

    let clampedA = clamp(in.v_width2.x, 0.0, (pattern_size_a.y + 2.0) * 0.5);
    let clampedB = clamp(in.v_width2.x, 0.0, (pattern_size_b.y + 2.0) * 0.5);

    let y_a = clamp(0.5 + (in.v_normal.y * clampedA / patternSizeAY), 0.0, 1.0);
    let y_b = clamp(0.5 + (in.v_normal.y * clampedB / patternSizeBY), 0.0, 1.0);

    let pos_a = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, vec2<f32>(x_a, y_a));
    let pos_b = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, vec2<f32>(x_b, y_b));

    let color_a = textureSample(pattern_texture, pattern_sampler, pos_a);
    let color_b = textureSample(pattern_texture, pattern_sampler, pos_b);

    let pattern_color = mix(color_a, color_b, tileProps.fade);

    return pattern_color * (alpha * opacity);
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
};

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
};

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
};

const LINE_EXPRESSION_COLOR: u32 = 1u << 0u;
const LINE_EXPRESSION_OPACITY: u32 = 1u << 1u;
const LINE_EXPRESSION_BLUR: u32 = 1u << 2u;
const LINE_EXPRESSION_WIDTH: u32 = 1u << 3u;
const LINE_EXPRESSION_GAPWIDTH: u32 = 1u << 4u;
const LINE_EXPRESSION_FLOORWIDTH: u32 = 1u << 5u;
const LINE_EXPRESSION_OFFSET: u32 = 1u << 6u;

struct LineSDFTilePropsUBO {
    sdfgamma: f32,
    mix: f32,
    pad1: f32,
    pad2: f32,
};

struct LineEvaluatedPropsUBO {
    color: vec4<f32>,
    blur: f32,
    opacity: f32,
    gapwidth: f32,
    offset: f32,
    width: f32,
    floorwidth: f32,
    expressionMask: u32,
    pad1: f32,
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct LineSDFDrawableEntry {
    data: LineSDFDrawableUBO,
};

struct LineSDFTilePropsEntry {
    data: LineSDFTilePropsUBO,
    pad0: vec4<f32>,
    pad1: vec4<f32>,
    pad2: vec4<f32>,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<LineSDFDrawableEntry>;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<LineSDFTilePropsEntry>;
@group(0) @binding(4) var<uniform> props: LineEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let index = globalIndex.value;
    let drawable = drawableVector[index].data;
    let tileProps = tilePropsVector[index].data;
    let mask = props.expressionMask;

    // Constants
    let LINE_DISTANCE_SCALE = 2.0;

    // Unpack vertex data
    let a_extrude = vec2<f32>(f32(in.data.x), f32(in.data.y)) - 128.0;
    let a_direction = f32(in.data.z % 4u) - 1.0;
    let v_linesofar = (floor(f32(in.data.z) * 0.25) + f32(in.data.w) * 64.0) * LINE_DISTANCE_SCALE;

    // Unpack position and normal
    let pos = floor(vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y)) * 0.5);
    let normal = vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y)) - 2.0 * pos;
    let v_normal = vec2<f32>(normal.x, normal.y * 2.0 - 1.0);

    // Unpack attributes using helper functions
    var color = unpack_mix_color(in.color, drawable.color_t);
    if ((mask & LINE_EXPRESSION_COLOR) != 0u) {
        color = props.color;
    }

    var blur = unpack_mix_float(in.blur, drawable.blur_t);
    if ((mask & LINE_EXPRESSION_BLUR) != 0u) {
        blur = props.blur;
    }

    var opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
    if ((mask & LINE_EXPRESSION_OPACITY) != 0u) {
        opacity = props.opacity;
    }

    var gapwidth = unpack_mix_float(in.gapwidth, drawable.gapwidth_t);
    if ((mask & LINE_EXPRESSION_GAPWIDTH) != 0u) {
        gapwidth = props.gapwidth;
    }
    gapwidth = gapwidth / 2.0;

    var offset = unpack_mix_float(in.offset, drawable.offset_t);
    if ((mask & LINE_EXPRESSION_OFFSET) != 0u) {
        offset = props.offset;
    }
    offset = -offset;

    var width = unpack_mix_float(in.width, drawable.width_t);
    if ((mask & LINE_EXPRESSION_WIDTH) != 0u) {
        width = props.width;
    }

    var floorwidth = unpack_mix_float(in.floorwidth, drawable.floorwidth_t);
    if ((mask & LINE_EXPRESSION_FLOORWIDTH) != 0u) {
        floorwidth = props.floorwidth;
    }

    let pixel_ratio = max(paintParams.pixel_ratio, 1e-6);
    let antialiasing = 0.5 / pixel_ratio;

    let halfwidth = width / 2.0;
    let inset = gapwidth + select(0.0, antialiasing, gapwidth > 0.0);
    let outset = gapwidth + halfwidth * select(1.0, 2.0, gapwidth > 0.0) +
                 select(0.0, antialiasing, halfwidth != 0.0);

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
        v_normal.y * drawable.patternscale_a.y + drawable.tex_y_a
    );
    let tex_b = vec2<f32>(
        v_linesofar * drawable.patternscale_b.x / floorwidth,
        v_normal.y * drawable.patternscale_b.y + drawable.tex_y_b
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
};

struct LineSDFTilePropsUBO {
    sdfgamma: f32,
    mix: f32,
    pad1: f32,
    pad2: f32,
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct LineSDFTilePropsEntry {
    data: LineSDFTilePropsUBO,
    pad0: vec4<f32>,
    pad1: vec4<f32>,
    pad2: vec4<f32>,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<LineSDFTilePropsEntry>;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(1) @binding(0) var sdf_sampler: sampler;
@group(1) @binding(1) var sdf_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value].data;

    // Calculate the distance of the pixel from the line
    let dist = length(in.v_normal) * in.v_width2.x;

    // Calculate the antialiasing fade factor
    let pixel_ratio = max(paintParams.pixel_ratio, 1e-6);
    let blur2 = (in.v_blur + 1.0 / pixel_ratio) * in.v_gamma_scale;
    let alpha = clamp(min(dist - (in.v_width2.y - blur2), in.v_width2.x - dist) / blur2, 0.0, 1.0);

    // Sample SDF texture
    let dist_a = textureSample(sdf_texture, sdf_sampler, in.v_tex_a).r;
    let dist_b = textureSample(sdf_texture, sdf_sampler, in.v_tex_b).r;
    let sdfdist = mix(dist_a, dist_b, tileProps.mix);

    // Calculate SDF alpha
    let sdf_alpha = smoothstep(0.5 - tileProps.sdfgamma / in.v_floorwidth, 0.5 + tileProps.sdfgamma / in.v_floorwidth, sdfdist);

    return in.v_color * (alpha * in.v_opacity * sdf_alpha);
}
)";
};

} // namespace shaders
} // namespace mbgl
