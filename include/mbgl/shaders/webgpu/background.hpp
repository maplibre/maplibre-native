#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "BackgroundShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(3) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
};

struct BackgroundDrawableUBO {
    matrix: mat4x4<f32>,
};

struct BackgroundDrawableUnionUBO {
    matrix_col0: vec4<f32>,
    matrix_col1: vec4<f32>,
    matrix_col2: vec4<f32>,
    matrix_col3: vec4<f32>,
    extra0: vec4<f32>,
    extra1: vec4<f32>,
};

struct BackgroundPropsUBO {
    color: vec4<f32>,
    opacity: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<BackgroundDrawableUnionUBO>;
@group(0) @binding(4) var<uniform> props: BackgroundPropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[globalIndex.value];
    let matrix = mat4x4<f32>(drawable.matrix_col0,
                             drawable.matrix_col1,
                             drawable.matrix_col2,
                             drawable.matrix_col3);
    let clip = matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    out.position = clip;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct BackgroundPropsUBO {
    color: vec4<f32>,
    opacity: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(4) var<uniform> props: BackgroundPropsUBO;

@fragment
fn main() -> @location(0) vec4<f32> {
    return props.color * props.opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "BackgroundPatternShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(3) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) pos_a: vec2<f32>,
    @location(1) pos_b: vec2<f32>,
};

struct BackgroundPatternDrawableUnionUBO {
    matrix_col0: vec4<f32>,
    matrix_col1: vec4<f32>,
    matrix_col2: vec4<f32>,
    matrix_col3: vec4<f32>,
    pixel_coord_upper: vec2<f32>,
    pixel_coord_lower: vec2<f32>,
    tile_units_to_pixels: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

struct BackgroundPatternPropsUBO {
    pattern_tl_a: vec2<f32>,
    pattern_br_a: vec2<f32>,
    pattern_tl_b: vec2<f32>,
    pattern_br_b: vec2<f32>,
    pattern_size_a: vec2<f32>,
    pattern_size_b: vec2<f32>,
    scale_a: f32,
    scale_b: f32,
    mix: f32,
    opacity: f32,
    pad1: f32,
    pad2: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<BackgroundPatternDrawableUnionUBO>;
@group(0) @binding(4) var<uniform> props: BackgroundPatternPropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let drawable = drawableVector[globalIndex.value];
    let matrix = mat4x4<f32>(drawable.matrix_col0,
                             drawable.matrix_col1,
                             drawable.matrix_col2,
                             drawable.matrix_col3);

    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));

    // Use get_pattern_pos helper function for pattern positioning
    out.pos_a = get_pattern_pos(
        drawable.pixel_coord_upper,
        drawable.pixel_coord_lower,
        props.scale_a * props.pattern_size_a,
        drawable.tile_units_to_pixels,
        pos
    );

    out.pos_b = get_pattern_pos(
        drawable.pixel_coord_upper,
        drawable.pixel_coord_lower,
        props.scale_b * props.pattern_size_b,
        drawable.tile_units_to_pixels,
        pos
    );

    let clip = matrix * vec4<f32>(pos, 0.0, 1.0);
    out.position = clip;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) pos_a: vec2<f32>,
    @location(1) pos_b: vec2<f32>,
};

struct BackgroundPatternPropsUBO {
    pattern_tl_a: vec2<f32>,
    pattern_br_a: vec2<f32>,
    pattern_tl_b: vec2<f32>,
    pattern_br_b: vec2<f32>,
    pattern_size_a: vec2<f32>,
    pattern_size_b: vec2<f32>,
    scale_a: f32,
    scale_b: f32,
    mix: f32,
    opacity: f32,
    pad1: f32,
    pad2: f32,
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    // other fields...
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(4) var<uniform> props: BackgroundPatternPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var pattern_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let texsize = paintParams.pattern_atlas_texsize;

    // Sample pattern A
    let imagecoord_a = gl_mod(in.pos_a, vec2<f32>(1.0));
    let pos_a = mix(props.pattern_tl_a / texsize, props.pattern_br_a / texsize, imagecoord_a);
    let color_a = textureSample(pattern_texture, texture_sampler, pos_a);

    // Sample pattern B
    let imagecoord_b = gl_mod(in.pos_b, vec2<f32>(1.0));
    let pos_b = mix(props.pattern_tl_b / texsize, props.pattern_br_b / texsize, imagecoord_b);
    let color_b = textureSample(pattern_texture, texture_sampler, pos_b);

    // Mix patterns and apply opacity
    return mix(color_a, color_b, props.mix) * props.opacity;
}
)";
};

} // namespace shaders
} // namespace mbgl
