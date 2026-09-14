#pragma once

#include <mln/shaders/fill_extrusion_shadow_layer_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

/// Shared by both shadow draw stages (roof, wall). The structs here must stay byte-identical to
/// the C++ ones in include/mln/shaders/fill_extrusion_shadow_layer_ubo.hpp.
constexpr auto fillExtrusionShadowShaderPrelude = R"(

enum {
    idFillExtrusionShadowDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idFillExtrusionShadowPropsUBO = drawableReservedUBOCount,
    fillExtrusionShadowUBOCount
};

struct alignas(16) FillExtrusionShadowDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float2 offset_per_meter;

    // Interpolations
    /* 72 */ float base_t;
    /* 76 */ float height_t;
    /* 80 */
};
static_assert(sizeof(FillExtrusionShadowDrawableUBO) == 5 * 16, "wrong size");

struct alignas(16) FillExtrusionShadowPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float opacity;
    /* 20 */ float base;
    /* 24 */ float height;
    /* 28 */ float pad1;
    /* 32 */
};
static_assert(sizeof(FillExtrusionShadowPropsUBO) == 2 * 16, "wrong size");

)";

//
// Shadow mask, roof triangles (non-instanced)

template <>
struct ShaderSource<BuiltIn::FillExtrusionShadowMaskShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionShadowMaskShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillExtrusionShadowShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(0)]];
    ushort2 decimals_ed [[attribute(1)]];

#if !defined(HAS_UNIFORM_u_base)
    float base [[attribute(2)]];
#endif
#if !defined(HAS_UNIFORM_u_height)
    float height [[attribute(3)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

struct FragmentOutput {
    half4 color [[color(0)]];
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillExtrusionShadowDrawableUBO* drawableVector [[buffer(idFillExtrusionShadowDrawableUBO)]],
                                device const FillExtrusionShadowPropsUBO& props [[buffer(idFillExtrusionShadowPropsUBO)]]) {

    device const FillExtrusionShadowDrawableUBO& drawable = drawableVector[uboIndex];

#if defined(HAS_UNIFORM_u_height)
    const auto height = props.height;
#else
    const auto height = max(unpack_mix_float(vertx.height, drawable.height_t), 0.0);
#endif

    // Roof triangles all sit at the top of the extrusion, exactly as FillExtrusionShader does with
    // its hardcoded t = 1.0.
    const float z = height;
    const float2 decimals = unpack_float(float(vertx.decimals_ed.x / 2)) / 128.0;
    const float2 p = float2(vertx.pos) + decimals;

    // Shear the vertex along the light ray onto the ground plane. This is the exact ground
    // projection of the point (p, z), not an approximation. `drawable.matrix` is the same
    // per-tile camera matrix the building itself is drawn with, so the shadow is positioned by
    // the same pipeline as the building.
    return {
        .position = drawable.matrix * float4(p + drawable.offset_per_meter * z, 0.0, 1.0),
    };
}

FragmentOutput fragment fragmentMain(FragmentStage in [[stage_in]],
                                     device const FillExtrusionShadowPropsUBO& props [[buffer(idFillExtrusionShadowPropsUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return { half4(1.0) };
#endif

    // Blending is alphaBlended(), i.e. (One, OneMinusSrcAlpha) -- premultiplied. mln::Color is
    // already premultiplied, so scaling the whole vector keeps rgb == straight_rgb * out_alpha.
    return { half4(props.color * props.opacity) };
}
)";
};

//
// Shadow mask, wall quads (instanced)

template <>
struct ShaderSource<BuiltIn::FillExtrusionShadowMaskInstancedShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionShadowMaskInstancedShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 4> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillExtrusionShadowShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    // Static unit quad: x in {0,1} selects p1 or p2, y in {0,1} is the lower/upper ring.
    short2 pos [[attribute(0)]];

#if !defined(HAS_UNIFORM_u_base)
    float base [[attribute(3)]];
#endif
#if !defined(HAS_UNIFORM_u_height)
    float height [[attribute(4)]];
#endif
};

// Must stay layout-identical to FillExtrusionLayoutVertex.
struct OutlineInstance {
    short2 pos;
    ushort2 decimals_ed;
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

struct FragmentOutput {
    half4 color [[color(0)]];
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillExtrusionShadowDrawableUBO* drawableVector [[buffer(idFillExtrusionShadowDrawableUBO)]],
                                device const FillExtrusionShadowPropsUBO& props [[buffer(idFillExtrusionShadowPropsUBO)]],
                                uint instanceID [[ instance_id ]],
                                device const OutlineInstance* outline [[buffer(fillExtrusionShadowUBOCount + 1)]]) {

    // The low bit of decimals_ed.x marks the last vertex of each ring. Skipping it also guards the
    // outline[instanceID + 1] read below from running off the end of the ring.
    bool isDiscarded = glMod(float(outline[instanceID].decimals_ed.x), 2.0) > 0.0;
    if (isDiscarded) {
        return { .position = float4(0.0) };
    }

    device const FillExtrusionShadowDrawableUBO& drawable = drawableVector[uboIndex];

#if defined(HAS_UNIFORM_u_base)
    const auto base   = props.base;
#else
    const auto base   = max(unpack_mix_float(vertx.base, drawable.base_t), 0.0);
#endif
#if defined(HAS_UNIFORM_u_height)
    const auto height = props.height;
#else
    const auto height = max(unpack_mix_float(vertx.height, drawable.height_t), 0.0);
#endif

    const float2 p1 = float2(outline[instanceID + 0].pos) + unpack_float(float(outline[instanceID + 0].decimals_ed.x / 2)) / 128.0;
    const float2 p2 = float2(outline[instanceID + 1].pos) + unpack_float(float(outline[instanceID + 1].decimals_ed.x / 2)) / 128.0;

    const float t = float(vertx.pos.y);

    // Shear by the vertex's own z, so a non-zero fill-extrusion-base correctly lifts the shadow's
    // near edge instead of anchoring it under the building.
    const float z = (t != 0.0) ? height : base;
    const float2 p = (vertx.pos.x == 0) ? p1 : p2;

    return {
        .position = drawable.matrix * float4(p + drawable.offset_per_meter * z, 0.0, 1.0),
    };
}

FragmentOutput fragment fragmentMain(FragmentStage in [[stage_in]],
                                     device const FillExtrusionShadowPropsUBO& props [[buffer(idFillExtrusionShadowPropsUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return { half4(1.0) };
#endif

    return { half4(props.color * props.opacity) };
}
)";
};

} // namespace shaders
} // namespace mln
