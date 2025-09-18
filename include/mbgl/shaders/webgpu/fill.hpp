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
// Include common functions
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
    @location(0) position: vec2<i32>,
    @location(1) color: vec4<f32>,
    @location(2) opacity: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
}

struct FillDrawableUBO {
    matrix: mat4x4<f32>,
    color_t: f32,
    opacity_t: f32,
    pad1: f32,
    pad2: f32,
}

struct FillDrawableUnionUBO {
    fill: FillDrawableUBO,
    padding: vec4<f32>,
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<FillDrawableUnionUBO>;

@vertex
fn main(@builtin(vertex_index) vertex_id: u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    // Transform position using the matrix
    let drawable = drawableVector[globalIndex.value].fill;
    let clip = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    let invW = 1.0 / clip.w;
    let ndcZ = (clip.z * invW) * 0.5 + 0.5;
    out.position = vec4<f32>(clip.x * invW,
                             -clip.y * invW,
                             ndcZ,
                             1.0);

    // Interpolate color and opacity
    out.color = unpack_mix_color(in.color, drawable.color_t);
    out.opacity = unpack_mix_float(in.opacity, drawable.opacity_t);

    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
}

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Use color and opacity from vertex shader
    return in.color * in.opacity;
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
// Include common functions
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
    @location(0) position: vec2<i32>,
    @location(1) outline_color: vec4<f32>,
    @location(2) opacity: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
}

struct FillOutlineDrawableUBO {
    matrix: mat4x4<f32>,
    outline_color_t: f32,
    opacity_t: f32,
    pad1: f32,
    pad2: f32,
}

struct FillOutlineDrawableUnionUBO {
    fill: FillOutlineDrawableUBO,
    padding: vec4<f32>,
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<FillOutlineDrawableUnionUBO>;

@vertex
fn main(@builtin(vertex_index) vertex_id: u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    // Transform position using the matrix
    let drawable = drawableVector[globalIndex.value].fill;
    let clip = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    let invW = 1.0 / clip.w;
    let ndcZ = (clip.z * invW) * 0.5 + 0.5;
    out.position = vec4<f32>(clip.x * invW,
                             -clip.y * invW,
                             ndcZ,
                             1.0);

    // Interpolate color and opacity
    out.color = unpack_mix_color(in.color, drawable.color_t);
    out.opacity = unpack_mix_float(in.opacity, drawable.opacity_t);

    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
}

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Use color and opacity from vertex shader
    return in.color * in.opacity;
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
    @location(5) pattern_from: vec4<u32>,
    @location(6) pattern_to: vec4<u32>,
    @location(7) opacity: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_pos_a: vec2<f32>,
    @location(1) v_pos_b: vec2<f32>,
    @location(2) pattern_from: vec4<f32>,
    @location(3) pattern_to: vec4<f32>,
    @location(4) opacity: f32,
}

struct FillPatternDrawableUBO {
    matrix: mat4x4<f32>,
    pixel_coord_upper: vec2<f32>,
    pixel_coord_lower: vec2<f32>,
    tile_ratio: f32,
    pattern_from_t: f32,
    pattern_to_t: f32,
    opacity_t: f32,
}

struct FillPatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    texsize: vec2<f32>,
    pad1: f32,
    pad2: f32,
}

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
}

struct GlobalPaintParamsUBO {
    pixel_ratio: f32,
    // other fields...
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

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

    let pattern_from = vec4<f32>(in.pattern_from);
    let pattern_to = vec4<f32>(in.pattern_to);

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
    let ndcZ = (clip.z * invW) * 0.5 + 0.5;
    out.position = vec4<f32>(clip.x * invW,
                             -clip.y * invW,
                             ndcZ,
                             1.0);
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
    out.pattern_from = pattern_from;
    out.pattern_to = pattern_to;
    out.opacity = unpack_mix_float(in.opacity, drawable.opacity_t);

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) v_pos_a: vec2<f32>,
    @location(1) v_pos_b: vec2<f32>,
    @location(2) pattern_from: vec4<f32>,
    @location(3) pattern_to: vec4<f32>,
    @location(4) opacity: f32,
}

struct FillPatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    texsize: vec2<f32>,
    pad1: f32,
    pad2: f32,
}

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

@group(0) @binding(4) var<storage, read> tilePropsVector: array<FillPatternTilePropsUBO>;
@group(0) @binding(5) var<uniform> props: FillEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var pattern_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value];

    let pattern_tl_a = in.pattern_from.xy;
    let pattern_br_a = in.pattern_from.zw;
    let pattern_tl_b = in.pattern_to.xy;
    let pattern_br_b = in.pattern_to.zw;

    // Sample pattern A
    let imagecoord_a = gl_mod(in.v_pos_a, vec2<f32>(1.0));
    let pos_a = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord_a);
    let color_a = textureSample(pattern_texture, texture_sampler, pos_a);

    // Sample pattern B
    let imagecoord_b = gl_mod(in.v_pos_b, vec2<f32>(1.0));
    let pos_b = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
    let color_b = textureSample(pattern_texture, texture_sampler, pos_b);

    // Mix patterns and apply opacity
    return mix(color_a, color_b, props.fade) * in.opacity;
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
    @location(5) pattern_from: vec4<u32>,
    @location(6) pattern_to: vec4<u32>,
    @location(7) opacity: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_pos_a: vec2<f32>,
    @location(1) v_pos_b: vec2<f32>,
    @location(2) v_pos: vec2<f32>,
    @location(3) pattern_from: vec4<f32>,
    @location(4) pattern_to: vec4<f32>,
    @location(5) opacity: f32,
}

struct FillOutlinePatternDrawableUBO {
    matrix: mat4x4<f32>,
    pixel_coord_upper: vec2<f32>,
    pixel_coord_lower: vec2<f32>,
    tile_ratio: f32,
    pattern_from_t: f32,
    pattern_to_t: f32,
    opacity_t: f32,
}

struct FillOutlinePatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    texsize: vec2<f32>,
    pad1: f32,
    pad2: f32,
}

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
}

struct GlobalPaintParamsUBO {
    pixel_ratio: f32,
    world_size: vec2<f32>,
    // other fields...
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

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

    let pattern_from = vec4<f32>(in.pattern_from);
    let pattern_to = vec4<f32>(in.pattern_to);

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
    let ndcZ = (clip.z * invW) * 0.5 + 0.5;

    out.position = vec4<f32>(ndcXY.x,
                             -ndcXY.y,
                             ndcZ,
                             1.0);
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
    out.pattern_from = pattern_from;
    out.pattern_to = pattern_to;
    out.opacity = unpack_mix_float(in.opacity, drawable.opacity_t);

    return out;
}
)";

    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) v_pos_a: vec2<f32>,
    @location(1) v_pos_b: vec2<f32>,
    @location(2) v_pos: vec2<f32>,
    @location(3) pattern_from: vec4<f32>,
    @location(4) pattern_to: vec4<f32>,
    @location(5) opacity: f32,
}

struct FillOutlinePatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    texsize: vec2<f32>,
    pad1: f32,
    pad2: f32,
}

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
}

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
}

@group(0) @binding(4) var<storage, read> tilePropsVector: array<FillOutlinePatternTilePropsUBO>;
@group(0) @binding(5) var<uniform> props: FillEvaluatedPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var pattern_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value];

    let pattern_tl_a = in.pattern_from.xy;
    let pattern_br_a = in.pattern_from.zw;
    let pattern_tl_b = in.pattern_to.xy;
    let pattern_br_b = in.pattern_to.zw;

    // Sample pattern A
    let imagecoord_a = gl_mod(in.v_pos_a, vec2<f32>(1.0));
    let pos_a = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord_a);
    let color_a = textureSample(pattern_texture, texture_sampler, pos_a);

    // Sample pattern B
    let imagecoord_b = gl_mod(in.v_pos_b, vec2<f32>(1.0));
    let pos_b = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
    let color_b = textureSample(pattern_texture, texture_sampler, pos_b);

    // Mix patterns and apply opacity
    return mix(color_a, color_b, props.fade) * in.opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillOutlineTriangulatedShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

} // namespace shaders
} // namespace mbgl
