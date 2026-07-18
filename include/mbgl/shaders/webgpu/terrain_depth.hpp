#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/terrain_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::TerrainDepthShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "TerrainDepthShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct TerrainDrawableUBO {
    matrix: mat4x4<f32>,
    dem_coords: vec4<f32>,
};

struct TerrainEvaluatedPropsUBO {
    unpack: vec4<f32>,
    exaggeration: f32,
    elevation_offset: f32,
    pad1: f32,
    pad2: f32,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<TerrainDrawableUBO>;
@group(0) @binding(4) var<uniform> props: TerrainEvaluatedPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var dem_texture: texture_2d<f32>;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[globalIndex.value];

    // Same elevation displacement as the terrain shader (webgpu/terrain.hpp),
    // rendering only depth for the symbol occlusion pass
    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));

    let elevation = get_elevation(pos, dem_texture, texture_sampler, drawable.dem_coords, props.unpack,
                                  drawable.dem_coords.w, props.exaggeration, 1.0);

    out.position = drawable.matrix * vec4<f32>(pos.x, pos.y, elevation, 1.0);
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @builtin(position) position: vec4<f32>,
};

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Pack the fragment depth into RGBA8, matching the maplibre-gl-js
    // terrain_depth shader; unpacked by unpack_depth() for calculate_visibility()
    let depth = in.position.z;
    let bit_shift = vec4<f32>(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
    let bit_mask = vec4<f32>(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
    var res = fract(depth * bit_shift);
    res = res - res.xxyz * bit_mask;
    return res;
}
)";
};

} // namespace shaders
} // namespace mbgl
