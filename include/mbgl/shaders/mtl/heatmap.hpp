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

    static constexpr AttributeInfo attributes[] = {
        {0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        {1, gfx::AttributeDataType::Float2, 1, "a_weight"},
        {2, gfx::AttributeDataType::Float2, 1, "a_radius"},
    };
    static constexpr UniformBlockInfo uniforms[] = {
        MLN_MTL_UNIFORM_BLOCK(3, true, false, HeatmapDrawableUBO),
        MLN_MTL_UNIFORM_BLOCK(4, true, true, HeatmapEvaluatedPropsUBO),
        MLN_MTL_UNIFORM_BLOCK(5, true, false, HeatmapInterpolateUBO),
        MLN_MTL_UNIFORM_BLOCK(6, true, true, HeatmapPermutationUBO),
        MLN_MTL_UNIFORM_BLOCK(7, true, false, ExpressionInputsUBO),
    };
    static constexpr TextureInfo textures[] = {};

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(0)]];
    float2 weight [[attribute(1)]];
    float2 radius [[attribute(2)]];
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

struct alignas(16) HeatmapPermutationUBO {
    Attribute weight;
    Attribute radius;
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
    float pad4, pad5, pad6;
};

// Effective "0" in the kernel density texture to adjust the kernel size to;
// this empirically chosen number minimizes artifacts on overlapping kernels
// for typical heatmap cases (assuming clustered source)
constant const float ZERO = 1.0 / 255.0 / 16.0;

// Gaussian kernel coefficient: 1 / sqrt(2 * PI)
#define GAUSS_COEF 0.3989422804014327

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const HeatmapDrawableUBO& drawable [[buffer(3)]],
                                device const HeatmapEvaluatedPropsUBO& props [[buffer(4)]],
                                device const HeatmapInterpolateUBO& interp [[buffer(5)]],
                                device const HeatmapPermutationUBO& permutation [[buffer(6)]],
                                device const ExpressionInputsUBO& expr [[buffer(7)]]) {

    const auto weight = valueFor(permutation.weight, props.weight, vertx.weight, interp.weight_t, expr);
    const auto radius = valueFor(permutation.radius, props.radius, vertx.radius, interp.radius_t, expr);
    
    // unencode the extrusion vector that we snuck into the a_pos vector
    float2 unscaled_extrude = float2(mod(float2(vertx.pos), 2.0) * 2.0 - 1.0);

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
    float S = sqrt(-2.0 * log(ZERO / weight / props.intensity / GAUSS_COEF)) / 3.0;

    // Pass the varying in units of radius
    float2 extrude = S * unscaled_extrude;

    // Scale by radius and the zoom-based scale factor to produce actual
    // mesh position
    float2 scaled_extrude = extrude * radius * drawable.extrude_scale;

    // multiply a_pos by 0.5, since we had it * 2 in order to sneak
    // in extrusion data
    float4 pos = float4(floor(float2(vertx.pos) * 0.5) + scaled_extrude, 0, 1);

    float4 position = drawable.matrix * pos;
    
    return {
        .position    = position,
        .weight      = weight,
        .extrude     = extrude,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const HeatmapEvaluatedPropsUBO& props [[buffer(4)]],
                            device const HeatmapPermutationUBO& permutation [[buffer(6)]]) {

    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    // Kernel density estimation with a Gaussian kernel of size 5x5
    float d = -0.5 * 3.0 * 3.0 * dot(in.extrude, in.extrude);
    float val = in.weight * props.intensity * GAUSS_COEF * exp(d);

    return half4(val, 1.0, 1.0, 1.0);
}
)";
};

} // namespace shaders
} // namespace mbgl
