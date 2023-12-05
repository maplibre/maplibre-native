#pragma once

#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "SymbolIconShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = true;

    static const std::array<AttributeInfo, 6> attributes;
    static const std::array<UniformBlockInfo, 7> uniforms;
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(
struct VertexStage {
    float4 pos_offset [[attribute(0)]];
    float4 data [[attribute(1)]];
    float4 pixeloffset [[attribute(2)]];
    float3 projected_pos [[attribute(3)]];
    float fade_opacity [[attribute(4)]];

#if !defined(HAS_UNIFORM_u_opacity)
    float opacity [[attribute(5)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 tex;
    half fade_opacity;

#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const SymbolDrawableUBO& drawable [[buffer(8)]],
                                device const SymbolDynamicUBO& dynamic [[buffer(9)]],
                                device const SymbolDrawablePaintUBO& paint [[buffer(10)]],
                                device const SymbolDrawableTilePropsUBO& props [[buffer(11)]],
                                device const SymbolDrawableInterpolateUBO& interp [[buffer(12)]],
                                device const SymbolPermutationUBO& permutation [[buffer(13)]],
                                device const ExpressionInputsUBO& expr [[buffer(14)]]) {

    const float2 a_pos = vertx.pos_offset.xy;
    const float2 a_offset = vertx.pos_offset.zw;

    const float2 a_tex = vertx.data.xy;
    const float2 a_size = vertx.data.zw;

    const float a_size_min = floor(a_size[0] * 0.5);
    const float2 a_pxoffset = vertx.pixeloffset.xy;
    const float2 a_minFontScale = vertx.pixeloffset.zw / 256.0;

    const float segment_angle = -vertx.projected_pos[2];

    float size;
    if (!props.is_size_zoom_constant && !props.is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], props.size_t) / 128.0;
    } else if (props.is_size_zoom_constant && !props.is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = props.size;
    }

    const float4 projectedPoint = drawable.matrix * float4(a_pos, 0, 1);
    const float camera_to_anchor_distance = projectedPoint.w;
    // See comments in symbol_sdf.vertex
    const float distance_ratio = props.pitch_with_map ?
        camera_to_anchor_distance / dynamic.camera_to_center_distance :
        dynamic.camera_to_center_distance / camera_to_anchor_distance;
    const float perspective_ratio = clamp(
            0.5 + 0.5 * distance_ratio,
            0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
            4.0);

    size *= perspective_ratio;

    const float fontScale = props.is_text ? size / 24.0 : size;

    float symbol_rotation = 0.0;
    if (drawable.rotate_symbol) {
        // See comments in symbol_sdf.vertex
        const float4 offsetProjectedPoint = drawable.matrix * float4(a_pos + float2(1, 0), 0, 1);

        const float2 a = projectedPoint.xy / projectedPoint.w;
        const float2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;
        symbol_rotation = atan2((b.y - a.y) / dynamic.aspect_ratio, b.x - a.x);
    }

    const float angle_sin = sin(segment_angle + symbol_rotation);
    const float angle_cos = cos(segment_angle + symbol_rotation);
    const float2x2 rotation_matrix = float2x2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);

    const float4 projected_pos = drawable.label_plane_matrix * float4(vertx.projected_pos.xy, 0.0, 1.0);
    const float2 pos0 = projected_pos.xy / projected_pos.w;
    const float2 posOffset = a_offset * max(a_minFontScale, fontScale) / 32.0 + a_pxoffset / 16.0;
    const float4 position = drawable.coord_matrix * float4(pos0 + rotation_matrix * posOffset, 0.0, 1.0);

    const float2 fade_opacity = unpack_opacity(vertx.fade_opacity);
    const float fade_change = fade_opacity[1] > 0.5 ? dynamic.fade_change : -dynamic.fade_change;

    return {
        .position     = position,
        .tex          = a_tex / drawable.texsize,
        .fade_opacity = half(max(0.0, min(1.0, fade_opacity[0] + fade_change))),
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity      = half(valueFor(permutation.opacity, paint.opacity, vertx.opacity, interp.opacity_t, expr)),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const SymbolDrawableUBO& drawable [[buffer(8)]],
                            device const SymbolDrawablePaintUBO& paint [[buffer(10)]],
                            device const SymbolPermutationUBO& permutation [[buffer(13)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

#if defined(HAS_UNIFORM_u_opacity)
    const half opacity = half(paint.opacity);
#else
    const half opacity = in.opacity;
#endif

    return half4(image.sample(image_sampler, in.tex) * (opacity * in.fade_opacity));
}
)";
};

} // namespace shaders
} // namespace mbgl
