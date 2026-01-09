#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillShader";
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
#ifndef HAS_UNIFORM_u_color
    @location(1) color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(2) opacity: vec2<f32>,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
};

struct FillDrawableUBO {
    matrix: mat4x4<f32>,
    color_t: f32,
    opacity_t: f32,
    pad1: f32,
    pad2: f32,
};

struct FillDrawableUnionUBO {
    fill: FillDrawableUBO,
    padding: vec4<f32>,
};

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<FillDrawableUnionUBO>;
@group(0) @binding(5) var<uniform> props: FillEvaluatedPropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    // Transform position using the matrix
    let drawable = drawableVector[globalIndex.value].fill;
    let clip = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    out.position = clip;

    var color: vec4<f32>;
#ifdef HAS_UNIFORM_u_color
    color = props.color;
#else
    color = unpack_mix_color(in.color, drawable.color_t);
#endif

    var opacity: f32;
#ifdef HAS_UNIFORM_u_opacity
    opacity = props.opacity;
#else
    opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
#endif

    out.color = color;
    out.opacity = opacity;

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
};

@group(0) @binding(5) var<uniform> props: FillEvaluatedPropsUBO;

struct FragmentInput {
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
};

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    var color: vec4<f32>;
#ifdef HAS_UNIFORM_u_color
    color = props.color;
#else
    color = in.color;
#endif

    var opacity: f32;
#ifdef HAS_UNIFORM_u_opacity
    opacity = props.opacity;
#else
    opacity = in.opacity;
#endif

    return color * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillOutlineShader";
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
#ifndef HAS_UNIFORM_u_outline_color
    @location(1) outline_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(2) opacity: vec2<f32>,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
    @location(2) pos: vec2<f32>,
};

struct FillOutlineDrawableUBO {
    matrix: mat4x4<f32>,
    outline_color_t: f32,
    opacity_t: f32,
    pad1: f32,
    pad2: f32,
};

struct FillOutlineDrawableUnionUBO {
    fill: FillOutlineDrawableUBO,
    padding: vec4<f32>,
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

struct FillOutlineEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<FillOutlineDrawableUnionUBO>;
@group(0) @binding(5) var<uniform> props: FillOutlineEvaluatedPropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    // Transform position using the matrix
    let drawable = drawableVector[globalIndex.value].fill;
    let clip = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    let invW = 1.0 / clip.w;
    let ndcXY = clip.xy * invW;
    out.position = clip;
    out.pos = (ndcXY + vec2<f32>(1.0)) * 0.5 * paintParams.world_size;

    var color: vec4<f32>;
#ifdef HAS_UNIFORM_u_outline_color
    color = props.outline_color;
#else
    color = unpack_mix_color(in.outline_color, drawable.outline_color_t);
#endif

    var opacity: f32;
#ifdef HAS_UNIFORM_u_opacity
    opacity = props.opacity;
#else
    opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
#endif

    out.color = color;
    out.opacity = opacity;

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FillOutlineEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
};

@group(0) @binding(5) var<uniform> props: FillOutlineEvaluatedPropsUBO;

struct FragmentInput {
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
    @location(2) pos: vec2<f32>,
};

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    var color: vec4<f32>;
#ifdef HAS_UNIFORM_u_outline_color
    color = props.outline_color;
#else
    color = in.color;
#endif

    var opacity: f32;
#ifdef HAS_UNIFORM_u_opacity
    opacity = props.opacity;
#else
    opacity = in.opacity;
#endif

    return color * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillPatternShader";
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(4) position: vec2<i32>,
#ifndef HAS_UNIFORM_u_pattern_from
    @location(5) pattern_from: vec4<u32>,
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    @location(6) pattern_to: vec4<u32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(7) opacity: vec2<f32>,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_pos_a: vec2<f32>,
    @location(1) v_pos_b: vec2<f32>,
#ifndef HAS_UNIFORM_u_pattern_from
    @location(2) pattern_from: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    @location(3) pattern_to: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(4) opacity: f32,
#endif
};

struct FillPatternDrawableUBO {
    matrix: mat4x4<f32>,
    pixel_coord_upper: vec2<f32>,
    pixel_coord_lower: vec2<f32>,
    tile_ratio: f32,
    pattern_from_t: f32,
    pattern_to_t: f32,
    opacity_t: f32,
};

struct FillPatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    texsize: vec2<f32>,
    pad1: f32,
    pad2: f32,
};

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
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

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<FillPatternDrawableUBO>;
@group(0) @binding(4) var<storage, read> tilePropsVector: array<FillPatternTilePropsUBO>;
@group(0) @binding(5) var<uniform> props: FillEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let index = globalIndex.value;
    let drawable = drawableVector[index];
    let tileProps = tilePropsVector[index];

    var pattern_from: vec4<f32>;
#ifdef HAS_UNIFORM_u_pattern_from
    pattern_from = tileProps.pattern_from;
#else
    pattern_from = vec4<f32>(in.pattern_from);
#endif

    var pattern_to: vec4<f32>;
#ifdef HAS_UNIFORM_u_pattern_to
    pattern_to = tileProps.pattern_to;
#else
    pattern_to = vec4<f32>(in.pattern_to);
#endif

    let pattern_tl_a = pattern_from.xy;
    let pattern_br_a = pattern_from.zw;
    let pattern_tl_b = pattern_to.xy;
    let pattern_br_b = pattern_to.zw;

    let pixelRatio = paintParams.pixel_ratio;
    let tileZoomRatio = drawable.tile_ratio;
    let fromScale = props.from_scale;
    let toScale = props.to_scale;

    let display_size_a = vec2<f32>(
        (pattern_br_a.x - pattern_tl_a.x) / pixelRatio,
        (pattern_br_a.y - pattern_tl_a.y) / pixelRatio
    );
    let display_size_b = vec2<f32>(
        (pattern_br_b.x - pattern_tl_b.x) / pixelRatio,
        (pattern_br_b.y - pattern_tl_b.y) / pixelRatio
    );

    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    let clip = drawable.matrix * vec4<f32>(pos, 0.0, 1.0);
    out.position = clip;
    out.v_pos_a = get_pattern_pos(
        drawable.pixel_coord_upper,
        drawable.pixel_coord_lower,
        fromScale * display_size_a,
        tileZoomRatio,
        pos
    );
    out.v_pos_b = get_pattern_pos(
        drawable.pixel_coord_upper,
        drawable.pixel_coord_lower,
        toScale * display_size_b,
        tileZoomRatio,
        pos
    );
#ifndef HAS_UNIFORM_u_pattern_from
    out.pattern_from = pattern_from;
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    out.pattern_to = pattern_to;
#endif
#ifndef HAS_UNIFORM_u_opacity
    out.opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
#endif

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) v_pos_a: vec2<f32>,
    @location(1) v_pos_b: vec2<f32>,
#ifndef HAS_UNIFORM_u_pattern_from
    @location(2) pattern_from: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    @location(3) pattern_to: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(4) opacity: f32,
#endif
};

struct FillPatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    texsize: vec2<f32>,
    pad1: f32,
    pad2: f32,
};

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(4) var<storage, read> tilePropsVector: array<FillPatternTilePropsUBO>;
@group(0) @binding(5) var<uniform> props: FillEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var pattern_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value];

    let pattern_from =
#ifdef HAS_UNIFORM_u_pattern_from
        tileProps.pattern_from;
#else
        in.pattern_from;
#endif

    let pattern_to =
#ifdef HAS_UNIFORM_u_pattern_to
        tileProps.pattern_to;
#else
        in.pattern_to;
#endif

    let pattern_tl_a = pattern_from.xy;
    let pattern_br_a = pattern_from.zw;
    let pattern_tl_b = pattern_to.xy;
    let pattern_br_b = pattern_to.zw;

    // Sample pattern A
    let imagecoord_a = gl_mod(in.v_pos_a, vec2<f32>(1.0));
    let pos_a = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord_a);
    let color_a = textureSample(pattern_texture, texture_sampler, pos_a);

    // Sample pattern B
    let imagecoord_b = gl_mod(in.v_pos_b, vec2<f32>(1.0));
    let pos_b = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
    let color_b = textureSample(pattern_texture, texture_sampler, pos_b);

    // Mix patterns and apply opacity
    let opacity =
#ifdef HAS_UNIFORM_u_opacity
        props.opacity;
#else
        in.opacity;
#endif

    return mix(color_a, color_b, props.fade) * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillOutlinePatternShader";
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(4) position: vec2<i32>,
#ifndef HAS_UNIFORM_u_pattern_from
    @location(5) pattern_from: vec4<u32>,
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    @location(6) pattern_to: vec4<u32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(7) opacity: vec2<f32>,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_pos_a: vec2<f32>,
    @location(1) v_pos_b: vec2<f32>,
    @location(2) v_pos: vec2<f32>,
#ifndef HAS_UNIFORM_u_pattern_from
    @location(3) pattern_from: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    @location(4) pattern_to: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(5) opacity: f32,
#endif
};

struct FillOutlinePatternDrawableUBO {
    matrix: mat4x4<f32>,
    pixel_coord_upper: vec2<f32>,
    pixel_coord_lower: vec2<f32>,
    tile_ratio: f32,
    pattern_from_t: f32,
    pattern_to_t: f32,
    opacity_t: f32,
};

struct FillOutlinePatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    texsize: vec2<f32>,
    pad1: f32,
    pad2: f32,
};

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
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

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<FillOutlinePatternDrawableUBO>;
@group(0) @binding(4) var<storage, read> tilePropsVector: array<FillOutlinePatternTilePropsUBO>;
@group(0) @binding(5) var<uniform> props: FillEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let index = globalIndex.value;
    let drawable = drawableVector[index];
    let tileProps = tilePropsVector[index];

    var pattern_from: vec4<f32>;
#ifdef HAS_UNIFORM_u_pattern_from
    pattern_from = tileProps.pattern_from;
#else
    pattern_from = vec4<f32>(in.pattern_from);
#endif

    var pattern_to: vec4<f32>;
#ifdef HAS_UNIFORM_u_pattern_to
    pattern_to = tileProps.pattern_to;
#else
    pattern_to = vec4<f32>(in.pattern_to);
#endif

    let pattern_tl_a = pattern_from.xy;
    let pattern_br_a = pattern_from.zw;
    let pattern_tl_b = pattern_to.xy;
    let pattern_br_b = pattern_to.zw;

    let pixelRatio = paintParams.pixel_ratio;
    let tileZoomRatio = drawable.tile_ratio;
    let fromScale = props.from_scale;
    let toScale = props.to_scale;

    let display_size_a = vec2<f32>(
        (pattern_br_a.x - pattern_tl_a.x) / pixelRatio,
        (pattern_br_a.y - pattern_tl_a.y) / pixelRatio
    );
    let display_size_b = vec2<f32>(
        (pattern_br_b.x - pattern_tl_b.x) / pixelRatio,
        (pattern_br_b.y - pattern_tl_b.y) / pixelRatio
    );

    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    let clip = drawable.matrix * vec4<f32>(pos, 0.0, 1.0);
    let invW = 1.0 / clip.w;
    let ndcXY = clip.xy * invW;

    out.position = clip;
    out.v_pos_a = get_pattern_pos(
        drawable.pixel_coord_upper,
        drawable.pixel_coord_lower,
        fromScale * display_size_a,
        tileZoomRatio,
        pos
    );
    out.v_pos_b = get_pattern_pos(
        drawable.pixel_coord_upper,
        drawable.pixel_coord_lower,
        toScale * display_size_b,
        tileZoomRatio,
        pos
    );
    out.v_pos = (ndcXY + vec2<f32>(1.0, 1.0)) * 0.5 * paintParams.world_size;
#ifndef HAS_UNIFORM_u_pattern_from
    out.pattern_from = pattern_from;
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    out.pattern_to = pattern_to;
#endif
#ifndef HAS_UNIFORM_u_opacity
    out.opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
#endif

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) v_pos_a: vec2<f32>,
    @location(1) v_pos_b: vec2<f32>,
    @location(2) v_pos: vec2<f32>,
#ifndef HAS_UNIFORM_u_pattern_from
    @location(3) pattern_from: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    @location(4) pattern_to: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(5) opacity: f32,
#endif
};

struct FillOutlinePatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    texsize: vec2<f32>,
    pad1: f32,
    pad2: f32,
};

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(4) var<storage, read> tilePropsVector: array<FillOutlinePatternTilePropsUBO>;
@group(0) @binding(5) var<uniform> props: FillEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var pattern_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value];

    let pattern_from =
#ifdef HAS_UNIFORM_u_pattern_from
        tileProps.pattern_from;
#else
        in.pattern_from;
#endif

    let pattern_to =
#ifdef HAS_UNIFORM_u_pattern_to
        tileProps.pattern_to;
#else
        in.pattern_to;
#endif

    let pattern_tl_a = pattern_from.xy;
    let pattern_br_a = pattern_from.zw;
    let pattern_tl_b = pattern_to.xy;
    let pattern_br_b = pattern_to.zw;

    // Sample pattern A
    let imagecoord_a = gl_mod(in.v_pos_a, vec2<f32>(1.0));
    let pos_a = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord_a);
    let color_a = textureSample(pattern_texture, texture_sampler, pos_a);

    // Sample pattern B
    let imagecoord_b = gl_mod(in.v_pos_b, vec2<f32>(1.0));
    let pos_b = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
    let color_b = textureSample(pattern_texture, texture_sampler, pos_b);

    // Mix patterns and apply opacity
    let opacity =
#ifdef HAS_UNIFORM_u_opacity
        props.opacity;
#else
        in.opacity;
#endif

    return mix(color_a, color_b, props.fade) * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillOutlineTriangulatedShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr const char* vertex = R"(
const SCALE: f32 = 0.015873016;

struct VertexInput {
    @location(0) pos_normal: vec2<i32>,
    @location(1) data: vec4<u32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) normal: vec2<f32>,
    @location(1) width: f32,
    @location(2) gamma_scale: f32,
};

struct FillOutlineTriangulatedDrawableUnionUBO {
    matrix_col0: vec4<f32>,
    matrix_col1: vec4<f32>,
    matrix_col2: vec4<f32>,
    matrix_col3: vec4<f32>,
    params: vec4<f32>,
    padding: vec4<f32>,
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

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<FillOutlineTriangulatedDrawableUnionUBO>;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let drawable = drawableVector[globalIndex.value];
    let matrix = mat4x4<f32>(drawable.matrix_col0,
                             drawable.matrix_col1,
                             drawable.matrix_col2,
                             drawable.matrix_col3);
    let ratio = max(drawable.params.x, 1e-6);

    let pos_normal = vec2<f32>(f32(in.pos_normal.x), f32(in.pos_normal.y));
    let pos = floor(pos_normal * 0.5);
    var normal = pos_normal - pos * 2.0;
    normal.y = normal.y * 2.0 - 1.0;

    let data = vec4<f32>(in.data);
    let extrude = data.xy - vec2<f32>(128.0, 128.0);

    let half_width = 0.5;
    let antialiasing = 0.5 / paintParams.pixel_ratio;
    let outset = half_width + antialiasing;

    let dist = extrude * outset * SCALE;
    let extrude_vec = dist / ratio;

    let projected_extrude = matrix * vec4<f32>(extrude_vec, 0.0, 0.0);
    let base = matrix * vec4<f32>(pos, 0.0, 1.0);
    let clip = base + projected_extrude;
    out.position = clip;

    let extrude_length_without_perspective = length(dist);
    let extrude_length_with_perspective = length((projected_extrude.xy / clip.w) * paintParams.units_to_pixels);
    let gamma_denom = max(extrude_length_with_perspective, 1e-6);

    out.normal = normal;
    out.width = outset;
    out.gamma_scale = extrude_length_without_perspective / gamma_denom;

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) normal: vec2<f32>,
    @location(1) width: f32,
    @location(2) gamma_scale: f32,
};

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
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
@group(0) @binding(5) var<uniform> props: FillEvaluatedPropsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let dist = length(in.normal) * in.width;
    let blur2 = (1.0 / paintParams.pixel_ratio) * in.gamma_scale;
    let denom = max(blur2, 1e-6);
    let alpha = clamp(min(dist + blur2, in.width - dist) / denom, 0.0, 1.0);
    return props.outline_color * (alpha * props.opacity);
}
)";
};

} // namespace shaders
} // namespace mbgl
