#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/webgpu/shader_program.hpp>
#include <mln/shaders/terrain_layer_ubo.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::TerrainShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "TerrainShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec4<i32>, // xy = tile position, z = skirt flag (1 = skirt)
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
    @location(1) elevation: f32,
    @location(2) depth: f32, // view depth, for the distance haze
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
    haze_color: vec4<f32>, // premultiplied; clear when there is no haze
    haze_ramp: vec2<f32>,  // haze = clamp(x - y / view depth, 0, 1)^2
    pad3: f32,
    pad4: f32,
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

    // Decode the DEM and interpolate in meters via the shared helper (the packed
    // Terrain-RGB/Terrarium DEM cannot be hardware-filtered, so it is sampled
    // NEAREST and interpolated after decoding, matching maplibre-gl-js and the
    // elevated layers). Map into the bound DEM tile; an ancestor tile is bound as
    // a fallback while this tile's own DEM loads. dem_coords.w = DEM dimension.
    let elevation = get_elevation(pos, dem_texture, texture_sampler, drawable.dem_coords, props.unpack,
                                  drawable.dem_coords.w, props.exaggeration, 1.0);

    // Skirt vertices hang below the surface by elevation_offset, forming a curtain
    // that hides the cracks between neighbouring tiles at different zoom levels
    // (maplibre-gl-js u_ele_delta). position.z carries the skirt flag.
    let ele_delta = select(0.0, props.elevation_offset, in.position.z == 1);
    out.position = drawable.matrix * vec4<f32>(pos.x, pos.y, elevation - ele_delta, 1.0);
    out.uv = uv;
    out.elevation = elevation;
    out.depth = out.position.w;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) uv: vec2<f32>,
    @location(1) elevation: f32,
    @location(2) depth: f32,
};

struct TerrainEvaluatedPropsUBO {
    unpack: vec4<f32>,
    exaggeration: f32,
    elevation_offset: f32,
    pad1: f32,
    pad2: f32,
    haze_color: vec4<f32>, // premultiplied; clear when there is no haze
    haze_ramp: vec2<f32>,  // haze = clamp(x - y / view depth, 0, 1)^2
    pad3: f32,
    pad4: f32,
};

@group(0) @binding(4) var<uniform> props: TerrainEvaluatedPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(2) var map_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Sample the map texture (render-to-texture output) for the surface color. WebGPU's
    // NDC is Y-up, as Metal's is, so the drape target is stored upright and uv.y is
    // sampled as it is. Flipping it (1.0 - uv.y, as Vulkan, whose NDC is Y-down, must)
    // turns the draped map upside down about each mesh tile.
    let color = textureSample(map_texture, texture_sampler, in.uv);

    // Distance haze: past where it begins the ground fades toward the haze color, as gl-js's terrain fog
    // does, to the full color (its alpha) where it ends (the ramp is TerrainLayerTweaker's). A zero
    // ramp leaves the drape as it is.
    var haze = clamp(props.haze_ramp.x - props.haze_ramp.y / max(in.depth, 1e-6), 0.0, 1.0);
    haze = haze * haze;
    return vec4<f32>(color.rgb * (1.0 - props.haze_color.a * haze) + props.haze_color.rgb * (color.a * haze),
                     color.a);
}
)";
};

} // namespace shaders
} // namespace mln
