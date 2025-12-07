#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto backgroundShaderPrelude = R"(

enum {
    idBackgroundDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idBackgroundPropsUBO = drawableReservedUBOCount,
    backgroundUBOCount
};

//
// Background

struct alignas(16) BackgroundDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(BackgroundDrawableUBO) == 4 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) BackgroundPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float opacity;
    /* 20 */ float pad1;
    /* 24 */ float pad2;
    /* 28 */ float pad3;
    /* 32 */
};
static_assert(sizeof(BackgroundPropsUBO) == 2 * 16, "wrong size");

//
// Background pattern

struct alignas(16) BackgroundPatternDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float2 pixel_coord_upper;
    /* 72 */ float2 pixel_coord_lower;
    /* 80 */ float tile_units_to_pixels;
    /* 84 */ float pad1;
    /* 88 */ float pad2;
    /* 92 */ float pad3;
    /* 96 */
};
static_assert(sizeof(BackgroundPatternDrawableUBO) == 6 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) BackgroundPatternPropsUBO {
    /*  0 */ float2 pattern_tl_a;
    /*  8 */ float2 pattern_br_a;
    /* 16 */ float2 pattern_tl_b;
    /* 24 */ float2 pattern_br_b;
    /* 32 */ float2 pattern_size_a;
    /* 40 */ float2 pattern_size_b;
    /* 48 */ float scale_a;
    /* 52 */ float scale_b;
    /* 56 */ float mix;
    /* 60 */ float opacity;
    /* 64 */
};
static_assert(sizeof(BackgroundPatternPropsUBO) == 4 * 16, "wrong size");

union BackgroundDrawableUnionUBO {
    BackgroundDrawableUBO backgroundDrawableUBO;
    BackgroundPatternDrawableUBO backgroundPatternDrawableUBO;
};

)";

template <>
struct ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "BackgroundShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = backgroundShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(backgroundUBOCount + 0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const BackgroundDrawableUnionUBO* drawableVector [[buffer(idBackgroundDrawableUBO)]]) {

    device const BackgroundDrawableUBO& drawable = drawableVector[uboIndex].backgroundDrawableUBO;

    return {
        .position = drawable.matrix * float4(float2(in.position.xy), 0, 1)
    };
}

PrecisionFloat4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const BackgroundPropsUBO& props [[buffer(idBackgroundPropsUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return PrecisionFloat4(1.0);
#endif

    return PrecisionFloat4(props.color * props.opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "BackgroundPatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = backgroundShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(backgroundUBOCount + 0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos_a;
    float2 pos_b;
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const BackgroundDrawableUnionUBO* drawableVector [[buffer(idBackgroundDrawableUBO)]],
                                device const BackgroundPatternPropsUBO& props [[buffer(idBackgroundPropsUBO)]]) {

    device const BackgroundPatternDrawableUBO& drawable = drawableVector[uboIndex].backgroundPatternDrawableUBO;

    const float2 pos = float2(in.position);
    const float2 pos_a = get_pattern_pos(drawable.pixel_coord_upper,
                                         drawable.pixel_coord_lower,
                                         props.scale_a * props.pattern_size_a,
                                         drawable.tile_units_to_pixels,
                                         pos);
    const float2 pos_b = get_pattern_pos(drawable.pixel_coord_upper,
                                         drawable.pixel_coord_lower,
                                         props.scale_b * props.pattern_size_b,
                                         drawable.tile_units_to_pixels,
                                         pos);
    return {
        .position = drawable.matrix * float4(float2(in.position.xy), 0, 1),
        .pos_a = pos_a,
        .pos_b = pos_b,
    };
}

PrecisionFloat4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParamsUBO [[buffer(idGlobalPaintParamsUBO)]],
                            device const BackgroundPatternPropsUBO& props [[buffer(idBackgroundPropsUBO)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return PrecisionFloat4(1.0);
#endif

    const float2 texsize = paintParamsUBO.pattern_atlas_texsize;
    const float2 imagecoord = glMod(float2(in.pos_a), 1.0);
    const float2 pos = mix(props.pattern_tl_a / texsize, props.pattern_br_a / texsize, imagecoord);
    const float4 color1 = image.sample(image_sampler, pos);
    const float2 imagecoord_b = glMod(float2(in.pos_b), 1.0);
    const float2 pos2 = mix(props.pattern_tl_b / texsize, props.pattern_br_b / texsize, imagecoord_b);
    const float4 color2 = image.sample(image_sampler, in.pos2);

    return PrecisionFloat4(mix(color1, color2, props.mix) * props.opacity);
}
)";
};

} // namespace shaders
} // namespace mbgl
