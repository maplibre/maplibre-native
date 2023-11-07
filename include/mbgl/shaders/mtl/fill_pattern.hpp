// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal> {


    static const ReflectionData reflectionData;
    static constexpr const char* sourceData = R"(struct VertexStage {
    short2 position [[attribute(0)]];
    ushort4 pattern_from [[attribute(1)]];
    ushort4 pattern_to [[attribute(2)]];
    float2 opacity [[attribute(3)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 pattern_from;
    float4 pattern_to;
    float2 v_pos_a;
    float2 v_pos_b;
    half opacity;
};

struct alignas(16) FillPatternDrawableUBO {
    float4x4 matrix;
    float4 scale;
    float2 pixel_coord_upper;
    float2 pixel_coord_lower;
    float2 texsize;
};

struct alignas(16) FillPatternEvaluatedPropsUBO {
    float opacity;
    float fade;
};

struct alignas(16) FillPatternTilePropsUBO {
    float4 pattern_from;
    float4 pattern_to;
};

struct alignas(16) FillPatternInterpolateUBO {
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
};

struct alignas(16) FillPatternPermutationUBO {
    Attribute pattern_from;
    Attribute pattern_to;
    Attribute opacity;
    bool overdrawInspector;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillPatternDrawableUBO& drawable [[buffer(4)]],
                                device const FillPatternTilePropsUBO& tileProps [[buffer(5)]],
                                device const FillPatternEvaluatedPropsUBO& props [[buffer(6)]],
                                device const FillPatternInterpolateUBO& interp [[buffer(7)]],
                                device const FillPatternPermutationUBO& permutation [[buffer(8)]],
                                device const ExpressionInputsUBO& expr [[buffer(9)]]) {

    const auto pattern_from   = patternFor(permutation.pattern_from, tileProps.pattern_from,  vertx.pattern_from,   interp.pattern_from_t,     expr);
    const auto pattern_to     = patternFor(permutation.pattern_to,   tileProps.pattern_to,    vertx.pattern_to,     interp.pattern_to_t,       expr);
    const auto opacity        = valueFor(permutation.opacity,        props.opacity,           vertx.opacity,        interp.opacity_t,        expr);

    float2 pattern_tl_a = pattern_from.xy;
    float2 pattern_br_a = pattern_from.zw;
    float2 pattern_tl_b = pattern_to.xy;
    float2 pattern_br_b = pattern_to.zw;

    float pixelRatio = drawable.scale.x;
    float tileZoomRatio = drawable.scale.y;
    float fromScale = drawable.scale.z;
    float toScale = drawable.scale.w;

    float2 display_size_a = float2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    float2 display_size_b = float2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);
    float2 postion = float2(vertx.position);

    return {
        .position       = drawable.matrix * float4(postion, 0, 1),
        .pattern_from   = pattern_from,
        .pattern_to     = pattern_to,
        .v_pos_a        = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, postion),
        .v_pos_b        = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, postion),
        .opacity        = half(opacity),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillPatternDrawableUBO& drawable [[buffer(4)]],
                            device const FillPatternEvaluatedPropsUBO& props [[buffer(6)]],
                            device const FillPatternPermutationUBO& permutation [[buffer(8)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    const float2 pattern_tl_a = in.pattern_from.xy;
    const float2 pattern_br_a = in.pattern_from.zw;
    const float2 pattern_tl_b = in.pattern_to.xy;
    const float2 pattern_br_b = in.pattern_to.zw;

    const float2 imagecoord = glMod(in.v_pos_a, 1.0);
    const float2 pos = mix(pattern_tl_a / drawable.texsize, pattern_br_a / drawable.texsize, imagecoord);
    const float4 color1 = image0.sample(image0_sampler, pos);

    const float2 imagecoord_b = glMod(in.v_pos_b, 1.0);
    const float2 pos2 = mix(pattern_tl_b / drawable.texsize, pattern_br_b / drawable.texsize, imagecoord_b);
    const float4 color2 = image0.sample(image0_sampler, pos2);

    return half4(mix(color1, color2, props.fade) * in.opacity);
}
)";
    static std::string source() {
        using Ty = ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>;
        return Ty::sourceData;
    }
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
