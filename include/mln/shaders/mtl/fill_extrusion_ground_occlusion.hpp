#pragma once

#include <mln/shaders/fill_extrusion_ground_occlusion_layer_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

/// Shared by both draws of the ground ambient occlusion. The structs here must stay byte-identical to the
/// C++ ones in include/mln/shaders/fill_extrusion_ground_occlusion_layer_ubo.hpp.
constexpr auto fillExtrusionGroundOcclusionShaderPrelude = R"(

enum {
    idFillExtrusionGroundOcclusionDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idFillExtrusionGroundOcclusionPropsUBO = drawableReservedUBOCount,
    fillExtrusionGroundOcclusionUBOCount
};

struct alignas(16) FillExtrusionGroundOcclusionDrawableUBO {
    /*  0 */ float4x4 matrix;

    // Interpolations
    /* 64 */ float base_t;
    /* 68 */ float height_t;

    /* 72 */ float meters_to_tile_units;
    /* 76 */ float radius;
    /* 80 */
};
static_assert(sizeof(FillExtrusionGroundOcclusionDrawableUBO) == 5 * 16, "wrong size");

struct alignas(16) FillExtrusionGroundOcclusionPropsUBO {
    /*  0 */ float intensity;
    /*  4 */ float base;
    /*  8 */ float height;
    /* 12 */ float pad1;
    /* 16 */
};
static_assert(sizeof(FillExtrusionGroundOcclusionPropsUBO) == 1 * 16, "wrong size");

)";

//
// Ground ambient occlusion around the footprints (instanced, one quad per outline edge)
//
// Each edge draws a capsule around itself: a quad extended by the radius past both ends and to
// both sides, fading out with the distance to the edge. Overlapping quads are combined with their
// maximum, so the result fades with the distance to the nearest wall. That takes two draws of the
// same quads: the first keeps the minimum of 1 - occlusion in the framebuffer's alpha, which is 1
// under the opaque map, and the second darkens the color by it and resets the alpha to 1, so any
// further overlapping quads leave it alone.

constexpr auto fillExtrusionGroundOcclusionShaderSource = R"(

struct VertexStage {
    // Static unit quad: x in {0,1} selects p1 or p2, y in {0,1} the inner or the outer side.
    short2 pos [[attribute(0)]];

#if !defined(HAS_UNIFORM_u_base)
    float2 base [[attribute(3)]];
#endif
#if !defined(HAS_UNIFORM_u_height)
    float2 height [[attribute(4)]];
#endif
};

// Must stay layout-identical to FillExtrusionLayoutVertex.
struct OutlineInstance {
    short2 pos;
    ushort2 decimals_ed;
};

struct FragmentStage {
    float4 position [[position, invariant]];
    // Signed distance from the wall, negative towards the inside, and the distances before the
    // start of the edge and past its end, negative along it. All in units of the radius.
    float3 distance;
    // How much the occlusion applies, 0 for parts that do not stand on the ground
    float strength;
};

struct FragmentOutput {
    half4 color [[color(0)]];
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillExtrusionGroundOcclusionDrawableUBO* drawableVector [[buffer(idFillExtrusionGroundOcclusionDrawableUBO)]],
                                device const FillExtrusionGroundOcclusionPropsUBO& props [[buffer(idFillExtrusionGroundOcclusionPropsUBO)]],
                                uint instanceID [[ instance_id ]],
                                device const OutlineInstance* outline [[buffer(fillExtrusionGroundOcclusionUBOCount + 1)]]) {

    // The low bit of decimals_ed.x marks the last vertex of each ring, which starts no edge
    if ((outline[instanceID].decimals_ed.x & 1) != 0) {
        return { .position = float4(0.0) };
    }

    device const FillExtrusionGroundOcclusionDrawableUBO& drawable = drawableVector[uboIndex];

#if defined(HAS_UNIFORM_u_base)
    const auto base = max(props.base, 0.0);
#else
    const auto base = max(unpack_mix_float(vertx.base, drawable.base_t), 0.0);
#endif
#if defined(HAS_UNIFORM_u_height)
    const auto height = max(props.height, 0.0);
#else
    const auto height = max(unpack_mix_float(vertx.height, drawable.height_t), 0.0);
#endif

    const float2 p1 = float2(outline[instanceID + 0].pos) + unpack_float(float(outline[instanceID + 0].decimals_ed.x / 2)) / 128.0;
    const float2 p2 = float2(outline[instanceID + 1].pos) + unpack_float(float(outline[instanceID + 1].decimals_ed.x / 2)) / 128.0;

    const float radius = drawable.radius * drawable.meters_to_tile_units;
    const float edgeLength = length(p2 - p1);
    // Flat footprints have no walls to occlude the ground
    if (radius <= 0.0 || edgeLength == 0.0 || height <= base) {
        return { .position = float4(0.0) };
    }
    const float2 along = (p2 - p1) / edgeLength;
    // The capsule covers both sides of the edge, so the winding does not matter. The half under
    // the building is mostly hidden by the depth test, but it covers the ground that shows through
    // cracks between the building's own triangles, which would stand out light against the
    // occlusion around it.
    const float2 outward = float2(along.y, -along.x);

    const bool atEnd = vertx.pos.x != 0;
    const float outer = vertx.pos.y != 0 ? 1.0 : -1.0;
    const float u = atEnd ? edgeLength / radius + 1.0 : -1.0;

    const float4 position = drawable.matrix * float4(p1 + (along * u + outward * outer) * radius, 0.0, 1.0);

    return {
        .position = position,
        .distance = float3(outer, -u, u - edgeLength / radius),
        .strength = ambientOcclusionFalloff(base / drawable.radius),
    };
}

// First draw: the minimum of 1 - occlusion goes into the alpha
FragmentOutput fragment fragmentMain(FragmentStage in [[stage_in]],
                                     device const FillExtrusionGroundOcclusionPropsUBO& props [[buffer(idFillExtrusionGroundOcclusionPropsUBO)]]) {
    const float occlusion = props.intensity * in.strength *
                            ambientOcclusionFalloff(length(float3(abs(in.distance.x), max(in.distance.yz, 0.0))));
    return { half4(0.0, 0.0, 0.0, 1.0 - occlusion) };
}

// Second draw, blended (One, DstAlpha): darkens the color by the alpha and resets it to 1
FragmentOutput fragment fragmentResolve(FragmentStage in [[stage_in]]) {
    return { half4(0.0, 0.0, 0.0, 1.0) };
}
)";

template <>
struct ShaderSource<BuiltIn::FillExtrusionGroundOcclusionShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionGroundOcclusionShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 4> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillExtrusionGroundOcclusionShaderPrelude;
    static constexpr auto source = fillExtrusionGroundOcclusionShaderSource;
};

template <>
struct ShaderSource<BuiltIn::FillExtrusionGroundOcclusionResolveShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionGroundOcclusionResolveShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentResolve";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 4> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillExtrusionGroundOcclusionShaderPrelude;
    static constexpr auto source = fillExtrusionGroundOcclusionShaderSource;
};

} // namespace shaders
} // namespace mln
