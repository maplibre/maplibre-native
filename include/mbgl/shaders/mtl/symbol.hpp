#pragma once

#include <mbgl/shaders/symbol_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto symbolShaderPrelude = R"(

enum {
    idSymbolDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idSymbolTilePropsUBO = idDrawableReservedFragmentOnlyUBO,
    idSymbolEvaluatedPropsUBO = drawableReservedUBOCount,
    symbolUBOCount
};

struct alignas(16) SymbolDrawableUBO {
    /*   0 */ float4x4 matrix;
    /*  64 */ float4x4 label_plane_matrix;
    /* 128 */ float4x4 coord_matrix;

    /* 192 */ float2 texsize;
    /* 200 */ float2 texsize_icon;

    /* 208 */ /*bool*/ int is_text_prop;
    /* 212 */ /*bool*/ int rotate_symbol;
    /* 216 */ /*bool*/ int pitch_with_map;
    /* 220 */ /*bool*/ int is_size_zoom_constant;
    /* 224 */ /*bool*/ int is_size_feature_constant;
    /* 228 */ /*bool*/ int is_offset;

    /* 232 */ float size_t;
    /* 236 */ float size;

    // Interpolations
    /* 240 */ float fill_color_t;
    /* 244 */ float halo_color_t;
    /* 248 */ float opacity_t;
    /* 252 */ float halo_width_t;
    /* 256 */ float halo_blur_t;
    /* 260 */
};
static_assert(sizeof(SymbolDrawableUBO) == 17 * 16, "wrong size");

struct alignas(16) SymbolTilePropsUBO {
    /*  0 */ /*bool*/ int is_text;
    /*  4 */ /*bool*/ int is_halo;
    /*  8 */ float gamma_scale;
    /* 12 */ float pad1;
    /* 16 */
};
static_assert(sizeof(SymbolTilePropsUBO) == 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) SymbolEvaluatedPropsUBO {
    /*  0 */ float4 text_fill_color;
    /* 16 */ float4 text_halo_color;
    /* 32 */ float text_opacity;
    /* 36 */ float text_halo_width;
    /* 40 */ float text_halo_blur;
    /* 44 */ float pad1;
    /* 48 */ float4 icon_fill_color;
    /* 64 */ float4 icon_halo_color;
    /* 80 */ float icon_opacity;
    /* 84 */ float icon_halo_width;
    /* 88 */ float icon_halo_blur;
    /* 92 */ float pad2;
    /* 96 */
};
static_assert(sizeof(SymbolEvaluatedPropsUBO) == 6 * 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "SymbolIconShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 6> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = symbolShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    float4 pos_offset [[attribute(symbolUBOCount + 0)]];
    float4 data [[attribute(symbolUBOCount + 1)]];
    float4 pixeloffset [[attribute(symbolUBOCount + 2)]];
    float3 projected_pos [[attribute(symbolUBOCount + 3)]];
    float fade_opacity [[attribute(symbolUBOCount + 4)]];

#if !defined(HAS_UNIFORM_u_opacity)
    float opacity [[attribute(symbolUBOCount + 5)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    half2 tex;

#if defined(HAS_UNIFORM_u_opacity)
    // We only need to pass `fade_opacity` separately if opacity is a
    // uniform, otherwise it's multiplied into fragment opacity, below.
    half fade_opacity;
#else
    half opacity;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const SymbolDrawableUBO* drawableVector [[buffer(idSymbolDrawableUBO)]]) {

    device const SymbolDrawableUBO& drawable = drawableVector[uboIndex];

    const float2 a_pos = vertx.pos_offset.xy;
    const float2 a_offset = vertx.pos_offset.zw;

    const float2 a_tex = vertx.data.xy;
    const float2 a_size = vertx.data.zw;

    const float a_size_min = floor(a_size[0] * 0.5);
    const float2 a_pxoffset = vertx.pixeloffset.xy;
    const float2 a_minFontScale = vertx.pixeloffset.zw / 256.0;

    const float segment_angle = -vertx.projected_pos[2];

    float size;
    if (!drawable.is_size_zoom_constant && !drawable.is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], drawable.size_t) / 128.0;
    } else if (drawable.is_size_zoom_constant && !drawable.is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = drawable.size;
    }

    const float4 projectedPoint = drawable.matrix * float4(a_pos, 0, 1);
    const float camera_to_anchor_distance = projectedPoint.w;
    // See comments in symbol_sdf.vertex
    const float distance_ratio = drawable.pitch_with_map ?
        camera_to_anchor_distance / paintParams.camera_to_center_distance :
        paintParams.camera_to_center_distance / camera_to_anchor_distance;
    const float perspective_ratio = clamp(
            0.5 + 0.5 * distance_ratio,
            0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
            4.0);

    if (!drawable.is_offset) {
        size *= perspective_ratio;
    }

    const float fontScale = drawable.is_text_prop ? size / 24.0 : size;

    float symbol_rotation = 0.0;
    if (drawable.rotate_symbol) {
        // See comments in symbol_sdf.vertex
        const float4 offsetProjectedPoint = drawable.matrix * float4(a_pos + float2(1, 0), 0, 1);

        const float2 a = projectedPoint.xy / projectedPoint.w;
        const float2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;
        symbol_rotation = atan2((b.y - a.y) / paintParams.aspect_ratio, b.x - a.x);
    }

    const float angle_sin = sin(segment_angle + symbol_rotation);
    const float angle_cos = cos(segment_angle + symbol_rotation);
    const float2x2 rotation_matrix = float2x2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);

    const float4 projected_pos = drawable.label_plane_matrix * float4(vertx.projected_pos.xy, 0.0, 1.0);
    const float2 pos0 = projected_pos.xy / projected_pos.w;
    const float2 posOffset = a_offset * max(a_minFontScale, fontScale) / 32.0 + a_pxoffset / 16.0;
    const float4 position = drawable.coord_matrix * float4(pos0 + rotation_matrix * posOffset, 0.0, 1.0);

    const float2 raw_fade_opacity = unpack_opacity(vertx.fade_opacity);
    const float fade_change = raw_fade_opacity[1] > 0.5 ? paintParams.symbol_fade_change : -paintParams.symbol_fade_change;
    const float fade_opacity = max(0.0, min(1.0, raw_fade_opacity[0] + fade_change));

    return {
        .position     = position,
        .tex          = half2(a_tex / drawable.texsize),
#if defined(HAS_UNIFORM_u_opacity)
        .fade_opacity = half(fade_opacity),
#else
        .opacity      = half(unpack_mix_float(vertx.opacity, drawable.opacity_t) * fade_opacity),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const SymbolTilePropsUBO* tilePropsVector [[buffer(idSymbolTilePropsUBO)]],
                            device const SymbolEvaluatedPropsUBO& props [[buffer(idSymbolEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const SymbolTilePropsUBO& tileProps = tilePropsVector[uboIndex];

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = (tileProps.is_text ? props.text_opacity : props.icon_opacity) * in.fade_opacity;
#else
    const float opacity = in.opacity; // fade_opacity is baked in for this case
#endif

    return half4(image.sample(image_sampler, float2(in.tex)) * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "SymbolSDFShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 10> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = symbolShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    float4 pos_offset [[attribute(symbolUBOCount + 0)]];
    float4 data [[attribute(symbolUBOCount + 1)]];
    float4 pixeloffset [[attribute(symbolUBOCount + 2)]];
    float3 projected_pos [[attribute(symbolUBOCount + 3)]];
    float fade_opacity [[attribute(symbolUBOCount + 4)]];

#if !defined(HAS_UNIFORM_u_fill_color)
    float4 fill_color [[attribute(symbolUBOCount + 5)]];
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
    float4 halo_color [[attribute(symbolUBOCount + 6)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity [[attribute(symbolUBOCount + 7)]];
#endif
#if !defined(HAS_UNIFORM_u_halo_width)
    float halo_width [[attribute(symbolUBOCount + 8)]];
#endif
#if !defined(HAS_UNIFORM_u_halo_blur)
    float halo_blur [[attribute(symbolUBOCount + 9)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];

#if !defined(HAS_UNIFORM_u_fill_color)
    half4 fill_color;
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
    half4 halo_color;
#endif

    half2 tex;
    half gamma_scale;
    half fontScale;
    half fade_opacity;

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
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const SymbolDrawableUBO* drawableVector [[buffer(idSymbolDrawableUBO)]]) {

    device const SymbolDrawableUBO& drawable = drawableVector[uboIndex];

    const float2 a_pos = vertx.pos_offset.xy;
    const float2 a_offset = vertx.pos_offset.zw;

    const float2 a_tex = vertx.data.xy;
    const float2 a_size = vertx.data.zw;

    const float a_size_min = floor(a_size[0] * 0.5);
    const float2 a_pxoffset = vertx.pixeloffset.xy;

    const float segment_angle = -vertx.projected_pos[2];

    float size;
    if (!drawable.is_size_zoom_constant && !drawable.is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], drawable.size_t) / 128.0;
    } else if (drawable.is_size_zoom_constant && !drawable.is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = drawable.size;
    }

    const float4 projectedPoint = drawable.matrix * float4(a_pos, 0, 1);
    const float camera_to_anchor_distance = projectedPoint.w;
    // If the label is pitched with the map, layout is done in pitched space,
    // which makes labels in the distance smaller relative to viewport space.
    // We counteract part of that effect by multiplying by the perspective ratio.
    // If the label isn't pitched with the map, we do layout in viewport space,
    // which makes labels in the distance larger relative to the features around
    // them. We counteract part of that effect by dividing by the perspective ratio.
    const float distance_ratio = drawable.pitch_with_map ?
        camera_to_anchor_distance / paintParams.camera_to_center_distance :
        paintParams.camera_to_center_distance / camera_to_anchor_distance;
    const float perspective_ratio = clamp(
        0.5 + 0.5 * distance_ratio,
        0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
        4.0);

    if (!drawable.is_offset) {
        size *= perspective_ratio;
    }

    const float fontScale = drawable.is_text_prop ? size / 24.0 : size;

    float symbol_rotation = 0.0;
    if (drawable.rotate_symbol) {
        // Point labels with 'rotation-alignment: map' are horizontal with respect to tile units
        // To figure out that angle in projected space, we draw a short horizontal line in tile
        // space, project it, and measure its angle in projected space.
        const float4 offsetProjectedPoint = drawable.matrix * float4(a_pos + float2(1, 0), 0, 1);

        const float2 a = projectedPoint.xy / projectedPoint.w;
        const float2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;

        symbol_rotation = atan2((b.y - a.y) / paintParams.aspect_ratio, b.x - a.x);
    }

    const float angle_sin = sin(segment_angle + symbol_rotation);
    const float angle_cos = cos(segment_angle + symbol_rotation);
    const auto rotation_matrix = float2x2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);
    const float4 projected_pos = drawable.label_plane_matrix * float4(vertx.projected_pos.xy, 0.0, 1.0);
    const float2 pos_rot = a_offset / 32.0 * fontScale + a_pxoffset;
    const float2 pos0 = projected_pos.xy / projected_pos.w + rotation_matrix * pos_rot;
    const float4 position = drawable.coord_matrix * float4(pos0, 0.0, 1.0);
    const float2 fade_opacity = unpack_opacity(vertx.fade_opacity);
    const float fade_change = (fade_opacity[1] > 0.5) ? paintParams.symbol_fade_change : -paintParams.symbol_fade_change;

    return {
        .position     = position,
#if !defined(HAS_UNIFORM_u_fill_color)
        .fill_color   = half4(unpack_mix_color(vertx.fill_color, drawable.fill_color_t)),
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
        .halo_color   = half4(unpack_mix_color(vertx.halo_color, drawable.halo_color_t)),
#endif
#if !defined(HAS_UNIFORM_u_halo_width)
        .halo_width   = half(unpack_mix_float(vertx.halo_width, drawable.halo_width_t)),
#endif
#if !defined(HAS_UNIFORM_u_halo_blur)
        .halo_blur    = half(unpack_mix_float(vertx.halo_blur, drawable.halo_blur_t)),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity      = half(unpack_mix_float(vertx.opacity, drawable.opacity_t)),
#endif
        .tex          = half2(a_tex / drawable.texsize),
        .gamma_scale  = half(position.w),
        .fontScale    = half(fontScale),
        .fade_opacity = half(max(0.0, min(1.0, fade_opacity[0] + fade_change))),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const SymbolTilePropsUBO* tilePropsVector [[buffer(idSymbolTilePropsUBO)]],
                            device const SymbolEvaluatedPropsUBO& props [[buffer(idSymbolEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const SymbolTilePropsUBO& tileProps = tilePropsVector[uboIndex];

#if defined(HAS_UNIFORM_u_fill_color)
    const half4 fill_color = half4(tileProps.is_text ? props.text_fill_color : props.icon_fill_color);
#else
    const half4 fill_color = in.fill_color;
#endif
#if defined(HAS_UNIFORM_u_halo_color)
    const half4 halo_color = half4(tileProps.is_text ? props.text_halo_color : props.icon_halo_color);
#else
    const half4 halo_color = in.halo_color;
#endif
#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = tileProps.is_text ? props.text_opacity : props.icon_opacity;
#else
    const float opacity = in.opacity;
#endif
#if defined(HAS_UNIFORM_u_halo_width)
    const float halo_width = tileProps.is_text ? props.text_halo_width : props.icon_halo_width;
#else
    const float halo_width = in.halo_width;
#endif
#if defined(HAS_UNIFORM_u_halo_blur)
    const float halo_blur = tileProps.is_text ? props.text_halo_blur : props.icon_halo_blur;
#else
    const float halo_blur = in.halo_blur;
#endif

    const float EDGE_GAMMA = 0.105 / DEVICE_PIXEL_RATIO;
    const float fontGamma = in.fontScale * tileProps.gamma_scale;
    const half4 color = tileProps.is_halo ? halo_color : fill_color;
    const float gamma = ((tileProps.is_halo ? (halo_blur * 1.19 / SDF_PX) : 0) + EDGE_GAMMA) / fontGamma;
    const float buff = tileProps.is_halo ? (6.0 - halo_width / in.fontScale) / SDF_PX : (256.0 - 64.0) / 256.0;
    const float dist = image.sample(image_sampler, float2(in.tex)).a;
    const float gamma_scaled = gamma * in.gamma_scale;
    const float alpha = smoothstep(buff - gamma_scaled, buff + gamma_scaled, dist);

    return half4(color * (alpha * opacity * in.fade_opacity));
}
)";
};

template <>
struct ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "SymbolTextAndIconShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto prelude = symbolShaderPrelude;
    static constexpr auto source = R"(

#define SDF 1.0
#define ICON 0.0

struct VertexStage {
    float4 pos_offset [[attribute(symbolUBOCount + 0)]];
    float4 data [[attribute(symbolUBOCount + 1)]];
    float3 projected_pos [[attribute(symbolUBOCount + 2)]];
    float fade_opacity [[attribute(symbolUBOCount + 3)]];

#if !defined(HAS_UNIFORM_u_fill_color)
    float4 fill_color [[attribute(symbolUBOCount + 4)]];
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
    float4 halo_color [[attribute(symbolUBOCount + 5)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity [[attribute(symbolUBOCount + 6)]];
#endif
#if !defined(HAS_UNIFORM_u_halo_width)
    float halo_width [[attribute(symbolUBOCount + 7)]];
#endif
#if !defined(HAS_UNIFORM_u_halo_blur)
    float halo_blur [[attribute(symbolUBOCount + 8)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];

#if !defined(HAS_UNIFORM_u_fill_color)
    half4 fill_color;
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
    half4 halo_color;
#endif

    half2 tex;

#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
#if !defined(HAS_UNIFORM_u_halo_width)
    half halo_width;
#endif
#if !defined(HAS_UNIFORM_u_halo_blur)
    half halo_blur;
#endif

    half gamma_scale;
    half fontScale;
    half fade_opacity;
    bool is_icon;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const SymbolDrawableUBO* drawableVector [[buffer(idSymbolDrawableUBO)]]) {

    device const SymbolDrawableUBO& drawable = drawableVector[uboIndex];

    const float2 a_pos = vertx.pos_offset.xy;
    const float2 a_offset = vertx.pos_offset.zw;

    const float2 a_tex = vertx.data.xy;
    const float2 a_size = vertx.data.zw;

    const float a_size_min = floor(a_size[0] * 0.5);
    const float is_sdf = a_size[0] - 2.0 * a_size_min;

    const float segment_angle = -vertx.projected_pos[2];

    float size;
    if (!drawable.is_size_zoom_constant && !drawable.is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], drawable.size_t) / 128.0;
    } else if (drawable.is_size_zoom_constant && !drawable.is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = drawable.size;
    }

    const float4 projectedPoint = drawable.matrix * float4(a_pos, 0, 1);
    const float camera_to_anchor_distance = projectedPoint.w;
    // If the label is pitched with the map, layout is done in pitched space,
    // which makes labels in the distance smaller relative to viewport space.
    // We counteract part of that effect by multiplying by the perspective ratio.
    // If the label isn't pitched with the map, we do layout in viewport space,
    // which makes labels in the distance larger relative to the features around
    // them. We counteract part of that effect by dividing by the perspective ratio.
    const float distance_ratio = drawable.pitch_with_map ?
        camera_to_anchor_distance / paintParams.camera_to_center_distance :
        paintParams.camera_to_center_distance / camera_to_anchor_distance;
    const float perspective_ratio = clamp(
        0.5 + 0.5 * distance_ratio,
        0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
        4.0);

    if (!drawable.is_offset) {
        size *= perspective_ratio;
    }

    const float fontScale = size / 24.0;

    float symbol_rotation = 0.0;
    if (drawable.rotate_symbol) {
        // Point labels with 'rotation-alignment: map' are horizontal with respect to tile units
        // To figure out that angle in projected space, we draw a short horizontal line in tile
        // space, project it, and measure its angle in projected space.
        const float4 offsetProjectedPoint = drawable.matrix * float4(a_pos + float2(1, 0), 0, 1);

        const float2 a = projectedPoint.xy / projectedPoint.w;
        const float2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;

        symbol_rotation = atan2((b.y - a.y) / paintParams.aspect_ratio, b.x - a.x);
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
    const float fade_change = (fade_opacity[1] > 0.5) ? paintParams.symbol_fade_change : -paintParams.symbol_fade_change;
    const bool is_icon = (is_sdf == ICON);

    return {
        .position     = position,
        .tex          = half2(a_tex / (is_icon ? drawable.texsize_icon : drawable.texsize)),
        .gamma_scale  = half(gamma_scale),
        .fontScale    = half(fontScale),
        .fade_opacity = half(max(0.0, min(1.0, fade_opacity[0] + fade_change))),
        .is_icon      = is_icon,

#if !defined(HAS_UNIFORM_u_fill_color)
        .fill_color = half(unpack_mix_color(vertx.fill_color, drawable.fill_color_t));
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
        .halo_color = half(unpack_mix_color(vertx.halo_color, drawable.halo_color_t));
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity    = half(unpack_mix_float(vertx.opacity, drawable.opacity_t));
#endif
#if !defined(HAS_UNIFORM_u_halo_width)
        .halo_width = half(unpack_mix_float(vertx.halo_width, drawable.halo_width_t));
#endif
#if !defined(HAS_UNIFORM_u_halo_blur)
        .halo_blur  = half(unpack_mix_float(vertx.halo_blur, drawable.halo_blur_t));
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const SymbolTilePropsUBO* tilePropsVector [[buffer(idSymbolTilePropsUBO)]],
                            device const SymbolEvaluatedPropsUBO& props [[buffer(idSymbolEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> glyph_image [[texture(0)]],
                            texture2d<float, access::sample> icon_image [[texture(1)]],
                            sampler glyph_sampler [[sampler(0)]],
                            sampler icon_sampler [[sampler(1)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const SymbolTilePropsUBO& tileProps = tilePropsVector[uboIndex];

#if defined(HAS_UNIFORM_u_fill_color)
    const half4 fill_color = half4(tileProps.is_text ? props.text_fill_color : props.icon_fill_color);
#else
    const half4 fill_color = in.fill_color;
#endif
#if defined(HAS_UNIFORM_u_halo_color)
    const half4 halo_color = half4(tileProps.is_text ? props.text_halo_color : props.icon_halo_color);
#else
    const half4 halo_color = in.halo_color;
#endif
#if defined(HAS_UNIFORM_u_opacity)
    const half opacity = half(tileProps.is_text ? props.text_opacity : props.icon_opacity);
#else
    const half opacity = in.opacity;
#endif
#if defined(HAS_UNIFORM_u_halo_width)
    const half halo_width = half(tileProps.is_text ? props.text_halo_width : props.icon_halo_width);
#else
    const half halo_width = in.halo_width;
#endif
#if defined(HAS_UNIFORM_u_halo_blur)
    const half halo_blur = half(tileProps.is_text ? props.text_halo_blur : props.icon_halo_blur);
#else
    const half halo_blur = in.halo_blur;
#endif

    if (in.is_icon) {
        const float alpha = opacity * in.fade_opacity;
        return half4(icon_image.sample(icon_sampler, float2(in.tex)) * alpha);
    }

    const float EDGE_GAMMA = 0.105 / DEVICE_PIXEL_RATIO;
    const half4 color = tileProps.is_halo ? halo_color : fill_color;
    const float fontGamma = in.fontScale * tileProps.gamma_scale;
    const float gamma = ((tileProps.is_halo ? (halo_blur * 1.19 / SDF_PX) : 0) + EDGE_GAMMA) / fontGamma;
    const float buff = tileProps.is_halo ? (6.0 - halo_width / in.fontScale) / SDF_PX : (256.0 - 64.0) / 256.0;
    const float dist = glyph_image.sample(glyph_sampler, float2(in.tex)).a;
    const float gamma_scaled = gamma * in.gamma_scale;
    const float alpha = smoothstep(buff - gamma_scaled, buff + gamma_scaled, dist);

    return half4(color * (alpha * opacity * in.fade_opacity));
}
)";
};

} // namespace shaders
} // namespace mbgl
