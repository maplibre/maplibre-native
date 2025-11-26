#pragma once

#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto hillshadePrepareShaderPrelude = R"(

enum {
    idHillshadePrepareDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idHillshadePrepareTilePropsUBO = drawableReservedUBOCount,
    hillshadePrepareUBOCount
};

struct alignas(16) HillshadePrepareDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(HillshadePrepareDrawableUBO) == 4 * 16, "wrong size");

struct alignas(16) HillshadePrepareTilePropsUBO {
    /*  0 */ float4 unpack;
    /* 16 */ float2 dimension;
    /* 24 */ float zoom;
    /* 28 */ float maxzoom;
    /* 32 */
};
static_assert(sizeof(HillshadePrepareTilePropsUBO) == 2 * 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "HillshadePrepareShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = hillshadePrepareShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(hillshadePrepareUBOCount + 0)]];
    short2 texture_pos [[attribute(hillshadePrepareUBOCount + 1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const HillshadePrepareDrawableUBO& drawable [[buffer(idHillshadePrepareDrawableUBO)]],
                                device const HillshadePrepareTilePropsUBO& tileProps [[buffer(idHillshadePrepareTilePropsUBO)]]) {

    const float4 position = drawable.matrix * float4(float2(vertx.pos), 0, 1);

    float2 epsilon = 1.0 / tileProps.dimension;
    float scale = (tileProps.dimension.x - 2.0) / tileProps.dimension.x;
    const float2 pos = (float2(vertx.texture_pos) / 8192.0) * scale + epsilon;

    return {
        .position    = position,
        .pos         = pos,
    };
}

float getElevation(float2 coord, float bias, texture2d<float, access::sample> image, sampler image_sampler, float4 unpack) {
    // Convert encoded elevation value to meters
    float4 data = image.sample(image_sampler, coord) * 255.0;
    data.a = -1.0;
    return dot(data, unpack);
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const HillshadePrepareTilePropsUBO& tileProps [[buffer(idHillshadePrepareTilePropsUBO)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    float2 epsilon = 1.0 / tileProps.dimension;
    float tileSize = tileProps.dimension.x - 2.0;

    // queried pixels (using Sobel operator kernel):
    // +-----------+
    // |   |   |   |
    // | a | b | c |
    // |   |   |   |
    // +-----------+
    // |   |   |   |
    // | d | e | f |
    // |   |   |   |
    // +-----------+
    // |   |   |   |
    // | g | h | i |
    // |   |   |   |
    // +-----------+
    float a = getElevation(in.pos + float2(-epsilon.x, -epsilon.y), 0.0, image, image_sampler, tileProps.unpack);
    float b = getElevation(in.pos + float2(0, -epsilon.y), 0.0, image, image_sampler, tileProps.unpack);
    float c = getElevation(in.pos + float2(epsilon.x, -epsilon.y), 0.0, image, image_sampler, tileProps.unpack);
    float d = getElevation(in.pos + float2(-epsilon.x, 0), 0.0, image, image_sampler, tileProps.unpack);
  //float e = getElevation(in.pos, 0.0, image, image_sampler, tileProps.unpack);
    float f = getElevation(in.pos + float2(epsilon.x, 0), 0.0, image, image_sampler, tileProps.unpack);
    float g = getElevation(in.pos + float2(-epsilon.x, epsilon.y), 0.0, image, image_sampler, tileProps.unpack);
    float h = getElevation(in.pos + float2(0, epsilon.y), 0.0, image, image_sampler, tileProps.unpack);
    float i = getElevation(in.pos + float2(epsilon.x, epsilon.y), 0.0, image, image_sampler, tileProps.unpack);

    // Convert the raw pixel-space derivative (slope) into world-space slope.
    // The conversion factor is: tileSize / (8 * meters_per_pixel).
    // meters_per_pixel is calculated as pow(2.0, 28.2562 - u_zoom).
    // The exaggeration factor is applied to scale the effect at lower zooms.
    float exaggeration = tileProps.zoom < 2.0 ? 0.4 : tileProps.zoom < 4.5 ? 0.35 : 0.3;

    float2 deriv = float2(
        (c + f + f + i) - (a + d + d + g),
        (g + h + h + i) - (a + b + b + c)
    ) * tileSize / pow(2.0, (tileProps.zoom - tileProps.maxzoom) * exaggeration + 28.2562 - tileProps.zoom);

    // Encode the derivative into the color channels (r and g)
    // The derivative is scaled from world-space slope to the range [0, 1] for texture storage.
    // The maximum possible world-space derivative is assumed to be 4 (hence division by 8.0).
    float4 color = clamp(float4(
        deriv.x / 8.0 + 0.5,
        deriv.y / 8.0 + 0.5,
        1.0,
        1.0), 0.0, 1.0);

    return half4(color);
}
)";
};

} // namespace shaders
} // namespace mbgl
