#pragma once

#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "SymbolSDFIconShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static constexpr AttributeInfo attributes[] = {
        // always attributes
        {0, gfx::AttributeDataType::Short4, 1, "a_pos_offset"},
        {1, gfx::AttributeDataType::UShort4, 1, "a_data"},
        {2, gfx::AttributeDataType::Short4, 1, "a_pixeloffset"},
        {3, gfx::AttributeDataType::Float3, 1, "a_projected_pos"},
        {4, gfx::AttributeDataType::Float, 1, "a_fade_opacity"},

        // sometimes uniforms
        {5, gfx::AttributeDataType::Float4, 1, "a_fill_color"},
        {6, gfx::AttributeDataType::Float4, 1, "a_halo_color"},
        {7, gfx::AttributeDataType::Float, 1, "a_opacity"},
        {8, gfx::AttributeDataType::Float, 1, "a_halo_width"},
        {9, gfx::AttributeDataType::Float, 1, "a_halo_blur"},
    };
    static constexpr UniformBlockInfo uniforms[] = {
        MLN_MTL_UNIFORM_BLOCK(10, true, true, SymbolDrawableUBO),
        MLN_MTL_UNIFORM_BLOCK(11, true, false, SymbolDrawablePaintUBO),
        MLN_MTL_UNIFORM_BLOCK(12, true, true, SymbolDrawableTilePropsUBO),
        MLN_MTL_UNIFORM_BLOCK(13, true, false, SymbolDrawableInterpolateUBO),
        MLN_MTL_UNIFORM_BLOCK(14, true, true, SymbolPermutationUBO),
        MLN_MTL_UNIFORM_BLOCK(15, true, false, ExpressionInputsUBO),
    };
    static constexpr TextureInfo textures[] = {
        {0, "u_texture"},
    };

    static constexpr auto source = R"(
struct VertexStage {
    float4 pos_offset [[attribute(0)]];
    float4 data [[attribute(1)]];
    float4 pixeloffset [[attribute(2)]];
    float3 projected_pos [[attribute(3)]];
    float fade_opacity [[attribute(4)]];

    float4 fill_color [[attribute(5)]];
    float4 halo_color [[attribute(6)]];
    float opacity [[attribute(7)]];
    float halo_width [[attribute(8)]];
    float halo_blur [[attribute(9)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 fill_color;
    float4 halo_color;
    float halo_width;
    float halo_blur;
    float opacity;
    float2 data0;
    float3 data1;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const SymbolDrawableUBO& drawable [[buffer(10)]],
                                device const SymbolDrawablePaintUBO& paint [[buffer(11)]],
                                device const SymbolDrawableTilePropsUBO& props [[buffer(12)]],
                                device const SymbolDrawableInterpolateUBO& interp [[buffer(13)]],
                                device const SymbolPermutationUBO& permutation [[buffer(14)]],
                                device const ExpressionInputsUBO& expr [[buffer(15)]]) {

    const auto fill_color = colorFor(permutation.fill_color, paint.fill_color, vertx.fill_color, interp.fill_color_t, expr);
    const auto halo_color = colorFor(permutation.halo_color, paint.halo_color, vertx.halo_color, interp.halo_color_t, expr);
    const auto opacity = valueFor(permutation.opacity, paint.opacity, vertx.opacity, interp.opacity_t, expr);
    const auto halo_width = valueFor(permutation.halo_width, paint.halo_width, vertx.halo_width, interp.halo_width_t, expr);
    const auto halo_blur = valueFor(permutation.halo_blur, paint.halo_blur, vertx.halo_blur, interp.halo_blur_t, expr);

    const float2 a_pos = vertx.pos_offset.xy;
    const float2 a_offset = vertx.pos_offset.zw;

    const float2 a_tex = vertx.data.xy;
    const float2 a_size = vertx.data.zw;

    const float a_size_min = floor(a_size[0] * 0.5);
    const float2 a_pxoffset = vertx.pixeloffset.xy;

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
    // If the label is pitched with the map, layout is done in pitched space,
    // which makes labels in the distance smaller relative to viewport space.
    // We counteract part of that effect by multiplying by the perspective ratio.
    // If the label isn't pitched with the map, we do layout in viewport space,
    // which makes labels in the distance larger relative to the features around
    // them. We counteract part of that effect by dividing by the perspective ratio.
    const float distance_ratio = props.pitch_with_map ?
        camera_to_anchor_distance / drawable.camera_to_center_distance :
        drawable.camera_to_center_distance / camera_to_anchor_distance;
    const float perspective_ratio = clamp(
        0.5 + 0.5 * distance_ratio,
        0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
        4.0);

    size *= perspective_ratio;

    const float fontScale = props.is_text ? size / 24.0 : size;

    float symbol_rotation = 0.0;
    if (drawable.rotate_symbol) {
        // Point labels with 'rotation-alignment: map' are horizontal with respect to tile units
        // To figure out that angle in projected space, we draw a short horizontal line in tile
        // space, project it, and measure its angle in projected space.
        const float4 offsetProjectedPoint = drawable.matrix * float4(a_pos + float2(1, 0), 0, 1);

        const float2 a = projectedPoint.xy / projectedPoint.w;
        const float2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;

        symbol_rotation = atan2((b.y - a.y) / drawable.aspect_ratio, b.x - a.x);
    }

    const float angle_sin = sin(segment_angle + symbol_rotation);
    const float angle_cos = cos(segment_angle + symbol_rotation);
    const float2x2 rotation_matrix = float2x2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);

    const float4 projected_pos = drawable.label_plane_matrix * float4(vertx.projected_pos.xy, 0.0, 1.0);
    const float2 pos_rot = a_offset / 32.0 * fontScale + a_pxoffset;
    const float2 pos0 = projected_pos.xy / projected_pos.w + rotation_matrix * pos_rot;
    const float4 position = drawable.coord_matrix * float4(pos0, 0.0, 1.0);
    const float gamma_scale = position.w;

    const float2 fade_opacity = unpack_opacity(vertx.fade_opacity);
    const float fade_change = (fade_opacity[1] > 0.5) ? drawable.fade_change : -drawable.fade_change;
    const float interpolated_fade_opacity = max(0.0, min(1.0, fade_opacity[0] + fade_change));

    return {
        .position   = position,
        .fill_color = fill_color,
        .halo_color = halo_color,
        .halo_width = halo_width,
        .halo_blur  = halo_blur,
        .opacity    = opacity,
        .data0      = a_tex / drawable.texsize,
        .data1      = float3(gamma_scale, size, interpolated_fade_opacity),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const SymbolDrawableUBO& drawable [[buffer(10)]],
                            device const SymbolDrawableTilePropsUBO& props [[buffer(12)]],
                            device const SymbolPermutationUBO& permutation [[buffer(14)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    const float EDGE_GAMMA = 0.105 / drawable.device_pixel_ratio;

    const float2 tex = in.data0.xy;
    const float gamma_scale = in.data1.x;
    const float size = in.data1.y;
    const float fade_opacity = in.data1[2];

    const float fontScale = props.is_text ? size / 24.0 : size;

    float4 color = in.fill_color;
    float gamma = EDGE_GAMMA / (fontScale * drawable.gamma_scale);
    float buff = (256.0 - 64.0) / 256.0;
    if (props.is_halo) {
        color = in.halo_color;
        gamma = (in.halo_blur * 1.19 / SDF_PX + EDGE_GAMMA) / (fontScale * drawable.gamma_scale);
        buff = (6.0 - in.halo_width / fontScale) / SDF_PX;
    }

    const float dist = image.sample(image_sampler, tex).a;
    const float gamma_scaled = gamma * gamma_scale;
    const float alpha = smoothstep(buff - gamma_scaled, buff + gamma_scaled, dist);

    return half4(color * (alpha * in.opacity * fade_opacity));
}
)";
};

} // namespace shaders
} // namespace mbgl
