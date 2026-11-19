#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/webgpu/shader_program.hpp>

namespace mln {
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

// Radians conversion
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

// Ambient occlusion where a wall meets the ground: the share of the ambient light taken away at a distance `s` from
// the corner, in units of the radius, 1 at the corner and 0 from the radius on. (1 - s)^1.5 follows the share of the
// rays that hit the other surface within the radius. The corner looks the same from both sides, so the walls and the
// ground around them use the same curve.
fn ambientOcclusionFalloff(s: f32) -> f32 {
    let t = 1.0 - clamp(s, 0.0, 1.0);
    return t * sqrt(t);
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
} // namespace mln
