#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/webgpu/shader_program.hpp>

namespace mln {
namespace shaders {

//
// Ground ambient occlusion around the footprints (instanced, one quad per outline edge)
//
// Each edge draws a capsule around itself: a quad extended by the radius past both ends and to
// both sides, fading out with the distance to the edge. Overlapping quads are combined with their
// maximum, so the result fades with the distance to the nearest wall. That takes two draws of the
// same quads: the first keeps the minimum of 1 - occlusion in the framebuffer's alpha, which is 1
// under the opaque map, and the second darkens the color by it and resets the alpha to 1, so any
// further overlapping quads leave it alone. See include/mln/shaders/mtl/fill_extrusion_ground_occlusion.hpp.
//
// Fill-extrusions are not instanced here, so there is no outline buffer. Instead there is one
// instance per vertex of the bucket's mesh, which reads that vertex and the next one. Each edge
// p1 -> p2 of a ring is a roof vertex at p1 followed by the first vertex of the edge's wall, at p2.
// Every other instance draws nothing.
//
// The structs here must stay layout-identical to the C++ ones in
// include/mln/shaders/fill_extrusion_ground_occlusion_layer_ubo.hpp, and the bindings must match
// idFillExtrusionGroundOcclusionDrawableUBO and idFillExtrusionGroundOcclusionPropsUBO, which
// src/mln/shaders/webgpu/fill_extrusion_ground_occlusion.cpp checks.

constexpr auto fillExtrusionGroundOcclusionVertexSource = R"(
struct VertexInput {
    // Static unit quad: x in {0,1} selects p1 or p2, y in {0,1} the inner or the outer side.
    @location(3) quad: vec2<i32>,
    // This vertex of the mesh and the next one
    @location(4) position: vec2<i32>,
    @location(5) decimals_ed: vec2<u32>,
    @location(6) normal2d: vec2<i32>,
    @location(7) next_position: vec2<i32>,
    @location(8) next_decimals_ed: vec2<u32>,
    @location(9) next_normal2d: vec2<i32>,
#ifndef HAS_UNIFORM_u_base
    @location(10) base: vec2<f32>,
#endif
#ifndef HAS_UNIFORM_u_height
    @location(11) height: vec2<f32>,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    // Signed distance from the wall, negative towards the inside, and the distances before the
    // start of the edge and past its end, negative along it. All in units of the radius.
    @location(0) distance: vec3<f32>,
    // How much the occlusion applies, 0 for parts that do not stand on the ground
    @location(1) strength: f32,
};

struct FillExtrusionGroundOcclusionDrawableUBO {
    matrix: mat4x4<f32>,
    // Interpolations
    base_t: f32,
    height_t: f32,
    meters_to_tile_units: f32,
    radius: f32,
};

struct FillExtrusionGroundOcclusionPropsUBO {
    intensity: f32,
    base: f32,
    height: f32,
    pad1: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<FillExtrusionGroundOcclusionDrawableUBO>;
@group(0) @binding(4) var<uniform> props: FillExtrusionGroundOcclusionPropsUBO;

fn discarded() -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4<f32>(0.0);
    out.distance = vec3<f32>(0.0);
    out.strength = 0.0;
    return out;
}

@vertex
fn main(in: VertexInput) -> VertexOutput {
    // Only a roof vertex followed by a wall vertex starts an edge
    let isRoof = in.normal2d.x == 0 && in.normal2d.y == 0;
    let nextIsWall = in.next_normal2d.x != 0 || in.next_normal2d.y != 0;
    if (!isRoof || !nextIsWall) {
        return discarded();
    }

    let drawable = drawableVector[globalIndex.value];

#ifndef HAS_UNIFORM_u_base
    let base = max(unpack_mix_float(in.base, drawable.base_t), 0.0);
#else
    let base = max(props.base, 0.0);
#endif

#ifndef HAS_UNIFORM_u_height
    let height = max(unpack_mix_float(in.height, drawable.height_t), 0.0);
#else
    let height = max(props.height, 0.0);
#endif

    let p1 = vec2<f32>(in.position) + unpack_float(f32(in.decimals_ed.x / 2u)) / 128.0;
    let p2 = vec2<f32>(in.next_position) + unpack_float(f32(in.next_decimals_ed.x / 2u)) / 128.0;

    let radius = drawable.radius * drawable.meters_to_tile_units;
    let edgeLength = length(p2 - p1);
    // Flat footprints have no walls to occlude the ground
    if (radius <= 0.0 || edgeLength == 0.0 || height <= base) {
        return discarded();
    }
    let along = (p2 - p1) / edgeLength;
    // The capsule covers both sides of the edge, so the winding does not matter. The half under
    // the building is mostly hidden by the depth test, but it covers the ground that shows through
    // cracks between the building's own triangles.
    let outward = vec2<f32>(along.y, -along.x);

    let outer = select(-1.0, 1.0, in.quad.y != 0);
    let u = select(-1.0, edgeLength / radius + 1.0, in.quad.x != 0);

    var out: VertexOutput;
    out.position = drawable.matrix * vec4<f32>(p1 + (along * u + outward * outer) * radius, 0.0, 1.0);
    out.distance = vec3<f32>(outer, -u, u - edgeLength / radius);
    out.strength = ambientOcclusionFalloff(base / drawable.radius);
    return out;
}
)";

template <>
struct ShaderSource<BuiltIn::FillExtrusionGroundOcclusionShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillExtrusionGroundOcclusionShader";
    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 8> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = fillExtrusionGroundOcclusionVertexSource;

    // First draw: the minimum of 1 - occlusion goes into the alpha
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) distance: vec3<f32>,
    @location(1) strength: f32,
};

struct FillExtrusionGroundOcclusionPropsUBO {
    intensity: f32,
    base: f32,
    height: f32,
    pad1: f32,
};

@group(0) @binding(4) var<uniform> props: FillExtrusionGroundOcclusionPropsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let occlusion = props.intensity * in.strength *
                    ambientOcclusionFalloff(length(vec3<f32>(abs(in.distance.x), max(in.distance.yz, vec2<f32>(0.0)))));
    return vec4<f32>(0.0, 0.0, 0.0, 1.0 - occlusion);
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillExtrusionGroundOcclusionResolveShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillExtrusionGroundOcclusionResolveShader";
    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 8> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = fillExtrusionGroundOcclusionVertexSource;

    // Second draw, blended (One, DstAlpha): darkens the color by the alpha and resets it to 1
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) distance: vec3<f32>,
    @location(1) strength: f32,
};

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    return vec4<f32>(0.0, 0.0, 0.0, 1.0);
}
)";
};

} // namespace shaders
} // namespace mln
