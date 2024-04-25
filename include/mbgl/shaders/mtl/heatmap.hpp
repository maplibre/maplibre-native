#pragma once

#include <mbgl/shaders/heatmap_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "HeatmapShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<UniformBlockInfo, 3> uniforms;
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(4)]];

#if !defined(HAS_UNIFORM_u_weight)
    float2 weight [[attribute(5)]];
#endif
#if !defined(HAS_UNIFORM_u_radius)
    float2 radius [[attribute(6)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float weight;
    float2 extrude;
};

struct alignas(16) HeatmapDrawableUBO {
    float4x4 matrix;
    float extrude_scale;
    float pad1;
    float2 pad2;
};

struct alignas(16) HeatmapEvaluatedPropsUBO {
    float weight;
    float radius;
    float intensity;
    float pad1;
};

struct alignas(16) HeatmapInterpolateUBO {
    float weight_t;
    float radius_t;
    float2 pad1;
};

// Effective "0" in the kernel density texture to adjust the kernel size to;
// this empirically chosen number minimizes artifacts on overlapping kernels
// for typical heatmap cases (assuming clustered source)
constant const float ZERO = 1.0 / 255.0 / 16.0;

// Gaussian kernel coefficient: 1 / sqrt(2 * PI)
#define GAUSS_COEF 0.3989422804014327

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const HeatmapDrawableUBO& drawable [[buffer(1)]],
                                device const HeatmapEvaluatedPropsUBO& props [[buffer(2)]],
                                device const HeatmapInterpolateUBO& interp [[buffer(3)]]) {

#if defined(HAS_UNIFORM_u_weight)
    const auto weight = props.weight;
#else
    const auto weight = unpack_mix_float(vertx.weight, interp.weight_t);
#endif
#if defined(HAS_UNIFORM_u_radius)
    const auto radius = props.radius;
#else
    const auto radius = unpack_mix_float(vertx.radius, interp.radius_t);
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
                            device const HeatmapEvaluatedPropsUBO& props [[buffer(2)]]) {
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
