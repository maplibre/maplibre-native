#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

// The enum values are already defined in layer_ubo.hpp
// WebGPU uses the same indices as Metal

template <>
struct ShaderSource<BuiltIn::Prelude, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "Prelude";

    // Common WGSL code that can be included in other shaders
    static constexpr auto prelude = R"(
// Constants
const PI: f32 = 3.14159265358979323846;
const LINE_NORMAL_SCALE: f32 = 1.0 / 63.0;  // 1.0 / (127 / 2)
const MAX_LINE_DISTANCE: f32 = 32767.0;
const SDF_PX: f32 = 8.0;

// WebGPU equivalent of GLSL mod function
fn glMod(x: f32, y: f32) -> f32 {
    return x - y * floor(x / y);
}

fn glMod2(x: vec2<f32>, y: f32) -> vec2<f32> {
    return x - y * floor(x / y);
}

fn glMod2v(x: vec2<f32>, y: vec2<f32>) -> vec2<f32> {
    return x - y * floor(x / y);
}

// GLSL compatibility helpers
fn gl_mod(x: vec2<f32>, y: vec2<f32>) -> vec2<f32> {
    return glMod2v(x, y);
}

// Sample the terrain elevation in meters at a tile-local coordinate, with manual
// bilinear interpolation on DEM pixel centers (the DEM has a 1px backfilled border),
// matching the maplibre-gl-js get_elevation() prelude function. Unlike gl-js (global
// terrain uniforms), MapLibre Native carries the DEM data per-drawable, so the DEM
// texture and dem_* values are passed in as arguments rather than read from globals.
fn get_elevation(pos: vec2<f32>,
                 dem: texture_2d<f32>,
                 dem_sampler: sampler,
                 dem_coords: vec4<f32>,
                 dem_unpack: vec4<f32>,
                 dem_dim: f32,
                 dem_exaggeration: f32,
                 dem_enabled: f32) -> f32 {
    if (dem_enabled == 0.0) {
        return 0.0;
    }
    let coord = (pos * dem_coords.x + dem_coords.yz) * dem_dim + 1.0;
    let f = fract(coord);
    let c = (floor(coord) + 0.5) / (dem_dim + 2.0);
    let d = 1.0 / (dem_dim + 2.0);
    var tl = textureSampleLevel(dem, dem_sampler, c, 0.0) * 255.0;
    tl.a = -1.0;
    var tr = textureSampleLevel(dem, dem_sampler, c + vec2<f32>(d, 0.0), 0.0) * 255.0;
    tr.a = -1.0;
    var bl = textureSampleLevel(dem, dem_sampler, c + vec2<f32>(0.0, d), 0.0) * 255.0;
    bl.a = -1.0;
    var br = textureSampleLevel(dem, dem_sampler, c + vec2<f32>(d, d), 0.0) * 255.0;
    br.a = -1.0;
    let elevation = mix(mix(dot(tl, dem_unpack), dot(tr, dem_unpack), f.x),
                        mix(dot(bl, dem_unpack), dot(br, dem_unpack), f.x),
                        f.y);
    return elevation * dem_exaggeration;
}

// Radians conversion
// Place a clip-space position computed with a tile-local drape matrix into the
// current terrain drape render target (see the GL prelude for the derivation).
// `matrix` carries the drawable's tile (z, x, y) in its unused third column and
// target_tile is GlobalPaintParamsUBO drape_tile (w != 0 while drawing into a
// drape target).
fn apply_drape_transform(clip: vec4<f32>, matrix: mat4x4<f32>, target_tile: vec4<f32>) -> vec4<f32> {
    if (target_tile.w == 0.0) {
        return clip;
    }
    let tile = matrix[2].xyz;
    let k = target_tile.x - tile.x; // target zoom - drawable zoom
    let scale = exp2(k);
    var offset: vec2<f32>;
    if (k >= 0.0) {
        offset = tile.yz * scale - target_tile.yz;
    } else {
        offset = (tile.yz - target_tile.yz * exp2(-k)) * scale;
    }
    var result = clip;
    result.x = clip.x * scale + (scale - 1.0 + 2.0 * offset.x);
    result.y = clip.y * scale + (1.0 - scale - 2.0 * offset.y);
    return result;
}

// Unpack a depth value packed by the terrain depth pass (webgpu/terrain_depth.hpp),
// converted to NDC z, as in the maplibre-gl-js prelude
fn unpack_depth(rgba_depth: vec4<f32>) -> f32 {
    let bit_shift = vec4<f32>(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
    return dot(rgba_depth, bit_shift) * 2.0 - 1.0;
}

// Whether a clip-space position is visible in front of the terrain, from the
// packed terrain depth texture, matching maplibre-gl-js calculate_visibility().
// WebGPU has y-up NDC but a top-left texture origin (like Metal, unlike GL), so
// the V coordinate is flipped here where the GL version does not flip.
fn calculate_visibility(pos: vec4<f32>,
                        depth_texture: texture_2d<f32>,
                        depth_sampler: sampler,
                        depth_enabled: f32) -> f32 {
    if (depth_enabled == 0.0) {
        return 1.0;
    }
    let uv = pos.xy / pos.w * 0.5 + 0.5;
    let depth = unpack_depth(textureSampleLevel(depth_texture, depth_sampler, vec2<f32>(uv.x, 1.0 - uv.y), 0.0));
    if (pos.z / pos.w > depth) {
        return 0.0;
    }
    return 1.0;
}

fn radians(degrees: f32) -> f32 {
    return PI * degrees / 180.0;
}

// Helper functions for unpacking data
fn unpack_float(packedValue: f32) -> vec2<f32> {
    let packedIntValue = i32(packedValue);
    let v0 = packedIntValue / 256;
    return vec2<f32>(f32(v0), f32(packedIntValue - v0 * 256));
}

fn unpack_opacity(packedOpacity: f32) -> vec2<f32> {
    let intOpacity = i32(packedOpacity) / 2;
    return vec2<f32>(f32(intOpacity) / 127.0, glMod(packedOpacity, 2.0));
}

// Decode a color that has been packed into two floats
fn decode_color(encoded: vec2<f32>) -> vec4<f32> {
    let e0 = unpack_float(encoded[0]) / 255.0;
    let e1 = unpack_float(encoded[1]) / 255.0;
    return vec4<f32>(e0.x, e0.y, e1.x, e1.y);
}

// Unpack and interpolate between two float values
fn unpack_mix_float(packedValue: vec2<f32>, t: f32) -> f32 {
    return mix(packedValue[0], packedValue[1], t);
}

// Unpack and interpolate between two colors
fn unpack_mix_color(packedColors: vec4<f32>, t: f32) -> vec4<f32> {
    let minColor = decode_color(vec2<f32>(packedColors[0], packedColors[1]));
    let maxColor = decode_color(vec2<f32>(packedColors[2], packedColors[3]));
    return mix(minColor, maxColor, t);
}

// Get pattern position for texture atlas lookups
fn get_pattern_pos(pixel_coord_upper: vec2<f32>, pixel_coord_lower: vec2<f32>,
                   pattern_size: vec2<f32>, tile_units_to_pixels: f32, pos: vec2<f32>) -> vec2<f32> {
    let offset_a = gl_mod(pixel_coord_upper, pattern_size) * 256.0;
    let offset_b = gl_mod(offset_a, pattern_size) * 256.0 + pixel_coord_lower;
    let offset = gl_mod(offset_b, pattern_size);
    return (tile_units_to_pixels * pos + offset) / pattern_size;
}
)";
};

} // namespace shaders
} // namespace mbgl
