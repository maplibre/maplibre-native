#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/terrain_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::TerrainShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "TerrainShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec2<i32>,
    @location(6) texcoord: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
    @location(1) elevation: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct TerrainDrawableUBO {
    matrix: mat4x4<f32>,
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

    // The mesh was generated with coordinates from 0 to EXTENT (8192)
    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    let uv = pos / 8192.0;

    // Sample the DEM texture and decode elevation in meters using the source's
    // unpack vector (supports Mapbox Terrain-RGB and Terrarium encodings)
    var dem_sample = textureSampleLevel(dem_texture, texture_sampler, uv, 0.0) * 255.0;
    dem_sample.a = -1.0;
    let elevation_meters = dot(dem_sample, props.unpack);

    let elevation = elevation_meters * props.exaggeration;

    out.position = drawable.matrix * vec4<f32>(pos.x, pos.y, elevation, 1.0);
    out.uv = uv;
    out.elevation = elevation;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) uv: vec2<f32>,
    @location(1) elevation: f32,
};

@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(2) var map_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Sample the map texture (render-to-texture output) for the surface color
    // Note: Y-coordinate is flipped (1.0 - y) to match OpenGL convention
    return textureSample(map_texture, texture_sampler, vec2<f32>(in.uv.x, 1.0 - in.uv.y));
}
)";
};

} // namespace shaders
} // namespace mbgl
