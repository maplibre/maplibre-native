#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "ColorReliefShader";
    // NOTE: Attribute and texture definitions should be placed here
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 3> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    // Corresponds to in_position (a_pos in GLSL)
    @location(0) position: vec2<i32>,
    // Corresponds to in_texture_position (a_texture_pos in GLSL)
    @location(1) texture_pos: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) frag_position: vec2<f32>, // Out texture coordinate (v_pos in GLSL)
};

// Global index UBO (used to index into storage buffers, common in MapLibre's WebGPU/Vulkan)
struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

// Drawable UBO (matches C++ structure)
struct ColorReliefDrawableUBO {
    matrix: mat4x4<f32>,
};

// Tile Props UBO (matches C++ structure)
struct ColorReliefTilePropsUBO {
    unpack: vec4<f32>,
    dimension: vec2<f32>,
    color_ramp_size: i32,
    pad_tile0: f32,
};

// Bindings for Drawables (Group 0)
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<ColorReliefDrawableUBO>;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<ColorReliefTilePropsUBO>; // Assuming TileProps is also indexed here

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[globalIndex.value];
    let tileProps = tilePropsVector[globalIndex.value];

    // Vertex transformation
    out.position = drawable.matrix * vec4<f32>(vec3<f32>(in.position), 0.0, 1.0);

    // Calculate texture coordinate (0-1 range, with 1-pixel border compensation)
    let a_pos = vec2<f32>(in.texture_pos);
    let epsilon = 1.0 / tileProps.dimension;
    let scale = (tileProps.dimension.x - 2.0) / tileProps.dimension.x;
    out.frag_position = (a_pos / 8192.0) * scale + epsilon;

    // Handle poles (clamping texture coordinate)
    // select(false_val, true_val, condition)
    out.frag_position.y = select(out.frag_position.y, 0.0, a_pos.y < -32767.5);
    out.frag_position.y = select(out.frag_position.y, 1.0, a_pos.y > 32766.5);

    return out;
}
)"_wgsl;

    static constexpr auto fragment = R"(
// Tile Props UBO (Must match vertex)
struct ColorReliefTilePropsUBO {
    unpack: vec4<f32>,
    dimension: vec2<f32>,
    color_ramp_size: i32,
    pad_tile0: f32,
};

// Evaluated Properties UBO
struct ColorReliefEvaluatedPropsUBO {
    opacity: f32,
    pad_eval0: f32,
    pad_eval1: f32,
    pad_eval2: f32,
};

// Global index UBO
struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

// Bindings (Group 0 for index/storage, Group 1 for Uniforms, Group 2 for Textures)
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<ColorReliefTilePropsUBO>;

// EvaluatedProps UBO (Group 1 - Layer UBOs)
@group(1) @binding(0) var<uniform> props: ColorReliefEvaluatedPropsUBO;

// Textures and Samplers (Group 2)
@group(2) @binding(0) var dem_texture: texture_2d<f32>;
@group(2) @binding(1) var dem_sampler: sampler;
@group(2) @binding(2) var elevation_stops_texture: texture_2d<f32>;
@group(2) @binding(3) var color_stops_texture: texture_2d<f32>;

// Helper to get elevation
fn getElevation(coord: vec2<f32>, unpack: vec4<f32>) -> f32 {
    let data = textureSample(dem_texture, dem_sampler, coord) * 255.0;
    // Set alpha to -1.0 for the dot product
    let data_with_alpha = vec4<f32>(data.rgb, -1.0);
    // Convert encoded elevation value to meters
    return dot(data_with_alpha, unpack);
}

// Helper to get an elevation stop value (assuming 1D texture mapped to 2D)
fn getElevationStop(stop: i32, color_ramp_size: i32) -> f32 {
    let x = (f32(stop) + 0.5) / f32(color_ramp_size);
    return textureSample(elevation_stops_texture, dem_sampler, vec2<f32>(x, 0.0)).r;
}

// Helper to get a color stop value
fn getColorStop(stop: i32, color_ramp_size: i32) -> vec4<f32> {
    let x = (f32(stop) + 0.5) / f32(color_ramp_size);
    return textureSample(color_stops_texture, dem_sampler, vec2<f32>(x, 0.0));
}

@fragment
fn main(in: VertexOutput) -> @location(0) vec4<f32> {
    let global_index = globalIndex.value;
    let tileProps = tilePropsVector[global_index];
    let el = getElevation(in.frag_position, tileProps.unpack);

    // --- Binary search to find surrounding elevation stops ---
    var r: i32 = (tileProps.color_ramp_size - 1);
    var l: i32 = 0;

    // Loop continues until r = l + 1
    loop {
        if (r - l <= 1) { break; }
        let m = (r + l) / 2;
        let el_m = getElevationStop(m, tileProps.color_ramp_size);
        if (el < el_m) {
            r = m;
        } else {
            l = m;
        }
    }

    // Get elevation values for interpolation bounds
    let el_l = getElevationStop(l, tileProps.color_ramp_size);
    let el_r = getElevationStop(r, tileProps.color_ramp_size);

    // Get the colors at the stops
    let color_l = getColorStop(l, tileProps.color_ramp_size);
    let color_r = getColorStop(r, tileProps.color_ramp_size);

    // Interpolate the color based on the elevation value
    let range = el_r - el_l;
    
    // t = clamp((el - el_l) / range, 0.0, 1.0)
    var t: f32 = select(0.0, (el - el_l) / range, range > 0.0);
    t = clamp(t, 0.0, 1.0);
    
    let final_color = mix(color_l, color_r, t);

    // Apply the layer opacity
    return vec4<f32>(final_color.rgb, final_color.a * props.opacity);
}
)"_wgsl;
};

} // namespace shaders
} // namespace mbgl
