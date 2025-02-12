#pragma once

#include <mbgl/shaders/location_indicator_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto locationIndicatorShaderPrelude = R"(

enum {
    idLocationIndicatorUBO = drawableReservedUBOCount,
    locationIndicatorUBOCount
};

struct alignas(16) LocationIndicatorDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float4 color;
    /* 80 */
};
static_assert(sizeof(LocationIndicatorDrawableUBO) == 5 * 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LocationIndicatorShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto prelude = locationIndicatorShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    float2 position [[attribute(locationIndicatorUBOCount + 0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const LocationIndicatorDrawableUBO& drawable [[buffer(idLocationIndicatorUBO)]]) {

    return {
        .position = drawable.matrix * float4(vertx.position, 1.0)
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const LocationIndicatorDrawableUBO& drawable [[buffer(idLocationIndicatorUBO)]]) {
    return half4(drawable.color);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LocationIndicatorTexturedShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LocationIndicatorTexturedShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = locationIndicatorShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    float2 position [[attribute(locationIndicatorUBOCount + 0)]];
    float2 uv [[attribute(locationIndicatorUBOCount + 1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 uv;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const LocationIndicatorDrawableUBO& drawable [[buffer(idLocationIndicatorUBO)]]) {

    return {
        .position = drawable.matrix * float4(vertx.position, 1.0),
        .uv = vertx.uv
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const LocationIndicatorDrawableUBO& drawable [[buffer(idLocationIndicatorUBO)]],
                            texture2d<float, access::sample> colorTexture [[texture(0)]]) {

    constexpr sampler sampler2d(coord::normalized, filter::linear);
    const float4 color = colorTexture.sample(sampler2d, in.uv);

    return half4(color);
}
)";
};

} // namespace shaders
} // namespace mbgl
