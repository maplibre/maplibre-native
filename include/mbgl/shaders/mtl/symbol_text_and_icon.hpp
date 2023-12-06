#pragma once

#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "SymbolTextAndIconShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = true;

    static const std::array<AttributeInfo, 9> attributes;
    static const std::array<UniformBlockInfo, 8> uniforms;
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto source = R"(
#define SDF 1.0
#define ICON 0.0

struct VertexStage {
    float4 pos_offset [[attribute(0)]];
    float4 data [[attribute(1)]];
    float3 projected_pos [[attribute(2)]];
    float fade_opacity [[attribute(3)]];

#if !defined(HAS_UNIFORM_u_fill_color)
    float4 fill_color [[attribute(4)]];
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
    float4 halo_color [[attribute(5)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity [[attribute(6)]];
#endif
#if !defined(HAS_UNIFORM_u_halo_width)
    float halo_width [[attribute(7)]];
#endif
#if !defined(HAS_UNIFORM_u_halo_blur)
    float halo_blur [[attribute(8)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 data0;
    float4 data1;

#if !defined(HAS_UNIFORM_u_fill_color)
    half4 fill_color;
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
    half4 halo_color;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
#if !defined(HAS_UNIFORM_u_halo_width)
    half halo_width;
#endif
#if !defined(HAS_UNIFORM_u_halo_blur)
    half halo_blur;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const MatrixUBO& matrix [[buffer(9)]],
                                device const SymbolDrawableUBO& drawable [[buffer(10)]],
                                device const SymbolDynamicUBO& dynamic [[buffer(11)]],
                                device const SymbolDrawablePaintUBO& paint [[buffer(12)]],
                                device const SymbolDrawableTilePropsUBO& props [[buffer(13)]],
                                device const SymbolDrawableInterpolateUBO& interp [[buffer(14)]],
                                device const SymbolPermutationUBO& permutation [[buffer(15)]],
                                device const ExpressionInputsUBO& expr [[buffer(16)]]) {

    const float2 a_pos = vertx.pos_offset.xy;
    const float2 a_offset = vertx.pos_offset.zw;

    const float2 a_tex = vertx.data.xy;
    const float2 a_size = vertx.data.zw;

    const float a_size_min = floor(a_size[0] * 0.5);
    const float is_sdf = a_size[0] - 2.0 * a_size_min;

    const float segment_angle = -vertx.projected_pos[2];

    float size;
    if (!props.is_size_zoom_constant && !props.is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], props.size_t) / 128.0;
    } else if (props.is_size_zoom_constant && !props.is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = props.size;
    }

    const float4 projectedPoint = matrix.matrix * float4(a_pos, 0, 1);
    const float camera_to_anchor_distance = projectedPoint.w;
    // If the label is pitched with the map, layout is done in pitched space,
    // which makes labels in the distance smaller relative to viewport space.
    // We counteract part of that effect by multiplying by the perspective ratio.
    // If the label isn't pitched with the map, we do layout in viewport space,
    // which makes labels in the distance larger relative to the features around
    // them. We counteract part of that effect by dividing by the perspective ratio.
    const float distance_ratio = props.pitch_with_map ?
        camera_to_anchor_distance / dynamic.camera_to_center_distance :
        dynamic.camera_to_center_distance / camera_to_anchor_distance;
    const float perspective_ratio = clamp(
        0.5 + 0.5 * distance_ratio,
        0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
        4.0);

    size *= perspective_ratio;

    const float fontScale = size / 24.0;

    float symbol_rotation = 0.0;
    if (drawable.rotate_symbol) {
        // Point labels with 'rotation-alignment: map' are horizontal with respect to tile units
        // To figure out that angle in projected space, we draw a short horizontal line in tile
        // space, project it, and measure its angle in projected space.
        const float4 offsetProjectedPoint = matrix.matrix * float4(a_pos + float2(1, 0), 0, 1);

        const float2 a = projectedPoint.xy / projectedPoint.w;
        const float2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;

        symbol_rotation = atan2((b.y - a.y) / dynamic.aspect_ratio, b.x - a.x);
    }

    const float angle_sin = sin(segment_angle + symbol_rotation);
    const float angle_cos = cos(segment_angle + symbol_rotation);
    const float2x2 rotation_matrix = float2x2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);

    const float4 projected_pos = drawable.label_plane_matrix * float4(vertx.projected_pos.xy, 0.0, 1.0);
    const float2 pos_rot = a_offset / 32.0 * fontScale;
    const float2 pos0 = projected_pos.xy / projected_pos.w + rotation_matrix * pos_rot;
    const float4 position = drawable.coord_matrix * float4(pos0, 0.0, 1.0);
    const float gamma_scale = position.w;

    const float2 fade_opacity = unpack_opacity(vertx.fade_opacity);
    const float fade_change = (fade_opacity[1] > 0.5) ? dynamic.fade_change : -dynamic.fade_change;
    const float interpolated_fade_opacity = max(0.0, min(1.0, fade_opacity[0] + fade_change));

    return {
        .position   = position,
        .data0      = float4(a_tex / drawable.texsize, a_tex / drawable.texsize_icon),
        .data1      = float4(gamma_scale, size, interpolated_fade_opacity, is_sdf),

#if !defined(HAS_UNIFORM_u_fill_color)
        .fill_color = half(colorFor(permutation.fill_color, paint.fill_color, vertx.fill_color, interp.fill_color_t, expr));
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
        .halo_color = half(colorFor(permutation.halo_color, paint.halo_color, vertx.halo_color, interp.halo_color_t, expr));
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity    = half(valueFor(permutation.opacity, paint.opacity, vertx.opacity, interp.opacity_t, expr));
#endif
#if !defined(HAS_UNIFORM_u_halo_width)
        .halo_width = half(valueFor(permutation.halo_width, paint.halo_width, vertx.halo_width, interp.halo_width_t, expr));
#endif
#if !defined(HAS_UNIFORM_u_halo_blur)
        .halo_blur  = half(valueFor(permutation.halo_blur, paint.halo_blur, vertx.halo_blur, interp.halo_blur_t, expr));
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const SymbolDrawableUBO& drawable [[buffer(10)]],
                            device const SymbolDynamicUBO& dynamic [[buffer(11)]],
                            device const SymbolDrawablePaintUBO& paint [[buffer(12)]],
                            device const SymbolDrawableTilePropsUBO& props [[buffer(13)]],
                            device const SymbolPermutationUBO& permutation [[buffer(15)]],
                            texture2d<float, access::sample> glyph_image [[texture(0)]],
                            texture2d<float, access::sample> icon_image [[texture(1)]],
                            sampler glyph_sampler [[sampler(0)]],
                            sampler icon_sampler [[sampler(1)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

#if defined(HAS_UNIFORM_u_fill_color)
    const half4 fill_color = half4(paint.fill_color);
#else
    const half4 fill_color = in.fill_color;
#endif
#if defined(HAS_UNIFORM_u_halo_color)
    const half4 halo_color = half4(paint.halo_color);
#else
    const half4 halo_color = in.halo_color;
#endif
#if defined(HAS_UNIFORM_u_opacity)
    const half opacity = half(paint.opacity);
#else
    const half opacity = in.opacity;
#endif
#if defined(HAS_UNIFORM_u_halo_width)
    const half halo_width = half(paint.halo_width);
#else
    const half halo_width = in.halo_width;
#endif
#if defined(HAS_UNIFORM_u_halo_blur)
    const half halo_blur = half(paint.halo_blur);
#else
    const half halo_blur = in.halo_blur;
#endif

    const float fade_opacity = in.data1[2];

    if (in.data1.w == ICON) {
        const float2 tex_icon = in.data0.zw;
        const float alpha = opacity * fade_opacity;
        return half4(icon_image.sample(icon_sampler, tex_icon) * alpha);
    }

    const float2 tex = in.data0.xy;
    const float EDGE_GAMMA = 0.105 / dynamic.device_pixel_ratio;
    const float gamma_scale = in.data1.x;
    const float size = in.data1.y;
    const float fontScale = size / 24.0;
    const half4 color = props.is_halo ? halo_color : fill_color;
    const float fontGamma = fontScale * drawable.gamma_scale;
    const float gamma = ((props.is_halo ? (halo_blur * 1.19 / SDF_PX) : 0) + EDGE_GAMMA) / fontGamma;
    const float buff = props.is_halo ? (6.0 - halo_width / fontScale) / SDF_PX : (256.0 - 64.0) / 256.0;
    const float dist = glyph_image.sample(glyph_sampler, tex).a;
    const float gamma_scaled = gamma * gamma_scale;
    const float alpha = smoothstep(buff - gamma_scaled, buff + gamma_scaled, dist);

    return half4(color * (alpha * opacity * fade_opacity));
}
)";
};

} // namespace shaders
} // namespace mbgl
