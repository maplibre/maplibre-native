#pragma once

#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "HillshadePrepareShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = false;

    static const std::array<AttributeInfo, 2> attributes;
    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(0)]];
    short2 texture_pos [[attribute(1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
};

struct alignas(16) HillshadePrepareDrawableUBO {
    float4x4 matrix;
    float4 unpack;
    float2 dimension;
    float zoom;
    float maxzoom;
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
    float pad4, pad5, pad6;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const HillshadePrepareDrawableUBO& drawable [[buffer(2)]]) {

    const float4 position = drawable.matrix * float4(float2(vertx.pos), 0, 1);

    float2 epsilon = 1.0 / drawable.dimension;
    float scale = (drawable.dimension.x - 2.0) / drawable.dimension.x;
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
    return dot(data, unpack) / 4.0;
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const HillshadePrepareDrawableUBO& drawable [[buffer(2)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {

    if (drawable.overdrawInspector) {
        return half4(1.0);
    }

    float2 epsilon = 1.0 / drawable.dimension;

    // queried pixels:
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

    float a = getElevation(in.pos + float2(-epsilon.x, -epsilon.y), 0.0, image, image_sampler, drawable.unpack);
    float b = getElevation(in.pos + float2(0, -epsilon.y), 0.0, image, image_sampler, drawable.unpack);
    float c = getElevation(in.pos + float2(epsilon.x, -epsilon.y), 0.0, image, image_sampler, drawable.unpack);
    float d = getElevation(in.pos + float2(-epsilon.x, 0), 0.0, image, image_sampler, drawable.unpack);
  //float e = getElevation(in.pos, 0.0, image, image_sampler, drawable.unpack);
    float f = getElevation(in.pos + float2(epsilon.x, 0), 0.0, image, image_sampler, drawable.unpack);
    float g = getElevation(in.pos + float2(-epsilon.x, epsilon.y), 0.0, image, image_sampler, drawable.unpack);
    float h = getElevation(in.pos + float2(0, epsilon.y), 0.0, image, image_sampler, drawable.unpack);
    float i = getElevation(in.pos + float2(epsilon.x, epsilon.y), 0.0, image, image_sampler, drawable.unpack);

    // here we divide the x and y slopes by 8 * pixel size
    // where pixel size (aka meters/pixel) is:
    // circumference of the world / (pixels per tile * number of tiles)
    // which is equivalent to: 8 * 40075016.6855785 / (512 * pow(2, u_zoom))
    // which can be reduced to: pow(2, 19.25619978527 - u_zoom)
    // we want to vertically exaggerate the hillshading though, because otherwise
    // it is barely noticeable at low zooms. to do this, we multiply this by some
    // scale factor pow(2, (u_zoom - u_maxzoom) * a) where a is an arbitrary value
    // Here we use a=0.3 which works out to the expression below. see
    // nickidlugash's awesome breakdown for more info
    // https://github.com/mapbox/mapbox-gl-js/pull/5286#discussion_r148419556
    float exaggeration = drawable.zoom < 2.0 ? 0.4 : drawable.zoom < 4.5 ? 0.35 : 0.3;

    float2 deriv = float2(
        (c + f + f + i) - (a + d + d + g),
        (g + h + h + i) - (a + b + b + c)
    ) /  pow(2.0, (drawable.zoom - drawable.maxzoom) * exaggeration + 19.2562 - drawable.zoom);

    float4 color = clamp(float4(
        deriv.x / 2.0 + 0.5,
        deriv.y / 2.0 + 0.5,
        1.0,
        1.0), 0.0, 1.0);

    return half4(color);
}
)";
};

} // namespace shaders
} // namespace mbgl
