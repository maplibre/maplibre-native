#pragma once

#include <mbgl/shaders/heatmap_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto heatmapShaderPrelude = R"(

enum {
    idHeatmapDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idHeatmapEvaluatedPropsUBO = drawableReservedUBOCount,
    heatmapUBOCount
};

struct alignas(16) HeatmapDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float extrude_scale;

    // Interpolations
    /* 68 */ float weight_t;
    /* 72 */ float radius_t;
    /* 76 */ float pad1;
    /* 80 */
};
static_assert(sizeof(HeatmapDrawableUBO) == 5 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) HeatmapEvaluatedPropsUBO {
    /*  0 */ float weight;
    /*  4 */ float radius;
    /*  8 */ float intensity;
    /* 12 */ float padding;
    /* 16 */
};
static_assert(sizeof(HeatmapEvaluatedPropsUBO) == 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "HeatmapShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = heatmapShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(heatmapUBOCount + 0)]];

#if !defined(HAS_UNIFORM_u_weight)
    float2 weight [[attribute(heatmapUBOCount + 1)]];
#endif
#if !defined(HAS_UNIFORM_u_radius)
    float2 radius [[attribute(heatmapUBOCount + 2)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float weight;
    float2 extrude;
};

// Effective "0" in the kernel density texture to adjust the kernel size to;
// this empirically chosen number minimizes artifacts on overlapping kernels
// for typical heatmap cases (assuming clustered source)
constant const float ZERO = 1.0 / 255.0 / 16.0;

// Gaussian kernel coefficient: 1 / sqrt(2 * PI)
#define GAUSS_COEF 0.3989422804014327

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const HeatmapDrawableUBO* drawableVector [[buffer(idHeatmapDrawableUBO)]],
                                device const HeatmapEvaluatedPropsUBO& props [[buffer(idHeatmapEvaluatedPropsUBO)]]) {

    device const HeatmapDrawableUBO& drawable = drawableVector[uboIndex];

#if defined(HAS_UNIFORM_u_weight)
    const auto weight = props.weight;
#else
    const auto weight = unpack_mix_float(vertx.weight, drawable.weight_t);
#endif
#if defined(HAS_UNIFORM_u_radius)
    const auto radius = props.radius;
#else
    const auto radius = unpack_mix_float(vertx.radius, drawable.radius_t);
#endif

    // unencode the extrusion vector that we snuck into the a_pos vector
    const float2 unscaled_extrude = float2(glMod(float2(vertx.pos), 2.0) * 2.0 - 1.0);

    // This 'extrude' comes in ranging from [-1, -1], to [1, 1].  We'll use
    // it to produce the vertices of a square mesh framing the point feature
    // we're adding to the kernel density texture.  We'll also pass it as
    // a varying, so that the fragment shader can determine the distance of
    // each fragment from the point feature.
    // Before we do so, we need to scale it up sufficiently so that the
    // kernel falls effectively to zero at the edge of the mesh.
    // That is, we want to know S such that
    // weight * u_intensity * GAUSS_COEF * exp(-0.5 * 3.0^2 * S^2) == ZERO
    // Which solves to:
    // S = sqrt(-2.0 * log(ZERO / (weight * u_intensity * GAUSS_COEF))) / 3.0
    const float S = sqrt(-2.0 * log(ZERO / weight / props.intensity / GAUSS_COEF)) / 3.0;

    // Pass the varying in units of radius
    const float2 extrude = S * unscaled_extrude;

    // Scale by radius and the zoom-based scale factor to produce actual
    // mesh position
    const float2 scaled_extrude = extrude * radius * drawable.extrude_scale;

    // multiply a_pos by 0.5, since we had it * 2 in order to sneak
    // in extrusion data
    const float4 pos = float4(floor(float2(vertx.pos) * 0.5) + scaled_extrude, 0, 1);

    return {
        .position    = drawable.matrix * pos,
        .weight      = weight,
        .extrude     = extrude,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const HeatmapEvaluatedPropsUBO& props [[buffer(idHeatmapEvaluatedPropsUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    // Kernel density estimation with a Gaussian kernel of size 5x5
    const float d = -0.5 * 3.0 * 3.0 * dot(in.extrude, in.extrude);
    const float val = in.weight * props.intensity * GAUSS_COEF * exp(d);

    return half4(val, 1.0, 1.0, 1.0);
}
)";
};

} // namespace shaders
} // namespace mbgl
