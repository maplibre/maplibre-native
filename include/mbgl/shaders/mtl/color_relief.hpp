#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto colorReliefShaderPrelude = R"(

enum {
    idColorReliefDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idColorReliefTilePropsUBO = drawableReservedUBOCount,
    idColorReliefEvaluatedPropsUBO = idDrawableReservedFragmentOnlyUBO,
    colorReliefUBOCount
};

struct alignas(16) ColorReliefDrawableUBO {
    /* 0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(ColorReliefDrawableUBO) == 4 * 16, "wrong size");

struct alignas(16) ColorReliefTilePropsUBO {
    /* 0 */ float4 unpack;
    /* 16 */ float2 dimension;
    /* 24 */ int u_color_ramp_size;
    /* 28 */ float padding;
    /* 32 */
};
static_assert(sizeof(ColorReliefTilePropsUBO) == 2 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) ColorReliefEvaluatedPropsUBO {
    /* 0 */ float u_opacity;
    /* 4 */ float pad_eval0;
    /* 8 */ float pad_eval1;
    /* 12 */ float pad_eval2;
    /* 16 */
};
static_assert(sizeof(ColorReliefEvaluatedPropsUBO) == 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "ColorReliefShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 3> textures;

    static constexpr auto prelude = colorReliefShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(colorReliefUBOCount + 0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 v_pos;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const ColorReliefDrawableUBO& drawable [[buffer(idColorReliefDrawableUBO)]],
                                device const ColorReliefTilePropsUBO& tileProps [[buffer(idColorReliefTilePropsUBO)]]) {

    FragmentStage out;
    out.position = drawable.matrix * float4(float2(vertx.pos), 0, 1);

    float2 epsilon = 1.0 / tileProps.dimension;
    float scale = (tileProps.dimension.x - 2.0) / tileProps.dimension.x;
    out.v_pos = (float2(vertx.pos) / 8192.0) * scale + epsilon;

    // Handle poles
    if (vertx.pos.y < -32767.5) out.v_pos.y = 0.0;
    if (vertx.pos.y > 32766.5) out.v_pos.y = 1.0;

    return out;
}

float getElevation(float2 coord, texture2d<float, access::sample> image, sampler image_sampler, device const ColorReliefTilePropsUBO& tileProps) {
    // Convert encoded elevation value to meters
    float4 data = image.sample(image_sampler, coord) * 255.0;
    data.a = -1.0;
    return dot(data, tileProps.unpack);
}

float getElevationStop(int stop, texture2d<float, access::sample> elevationStops, sampler elevationStops_sampler, device const ColorReliefTilePropsUBO& tileProps) {
    // Elevation stops are plain float values, not terrain-RGB encoded
    float x = (float(stop) + 0.5) / float(tileProps.u_color_ramp_size);
    return elevationStops.sample(elevationStops_sampler, float2(x, 0.0)).r;
}

float4 getColorStop(int stop, texture2d<float, access::sample> colorStops, sampler colorStops_sampler, device const ColorReliefTilePropsUBO& tileProps) {
    float x = (float(stop) + 0.5) / float(tileProps.u_color_ramp_size);
    return colorStops.sample(colorStops_sampler, float2(x, 0.0));
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const ColorReliefTilePropsUBO& tileProps [[buffer(idColorReliefTilePropsUBO)]],
                            device const ColorReliefEvaluatedPropsUBO& props [[buffer(idColorReliefEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]],
                            texture2d<float, access::sample> elevationStops [[texture(1)]],
                            sampler elevationStops_sampler [[sampler(1)]],
                            texture2d<float, access::sample> colorStops [[texture(2)]],
                            sampler colorStops_sampler [[sampler(2)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    float el = getElevation(in.v_pos, image, image_sampler, tileProps);

    // Binary search for color stops
    int r = (tileProps.u_color_ramp_size - 1);
    int l = 0;

    while (r - l > 1) {
        int m = (r + l) / 2;
        float el_m = getElevationStop(m, elevationStops, elevationStops_sampler, tileProps);
        if (el < el_m) {
            r = m;
        } else {
            l = m;
        }
    }

    // Get elevation values for interpolation
    float el_l = getElevationStop(l, elevationStops, elevationStops_sampler, tileProps);
    float el_r = getElevationStop(r, elevationStops, elevationStops_sampler, tileProps);

    // Get colors for interpolation
    float4 color_l = getColorStop(l, colorStops, colorStops_sampler, tileProps);
    float4 color_r = getColorStop(r, colorStops, colorStops_sampler, tileProps);

    // Interpolate between the two colors
    float t = clamp((el - el_l) / (el_r - el_l), 0.0, 1.0);
    float4 color = props.u_opacity * mix(color_l, color_r, t);

    return half4(color);
}
)";
};

} // namespace shaders
} // namespace mbgl
