#pragma once

#include <mbgl/shaders/terrain_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/mtl/terrain.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::TerrainDepthShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "TerrainDepthShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    // Shares the terrain shader's UBO layout (same drawable/props buffers)
    static constexpr auto prelude = terrainShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const TerrainDrawableUBO* drawableVector [[buffer(idTerrainDrawableUBO)]],
                                device const TerrainEvaluatedPropsUBO& props [[buffer(idTerrainEvaluatedPropsUBO)]],
                                texture2d<float, access::sample> demTexture [[texture(0)]],
                                sampler demSampler [[sampler(0)]]) {

    device const TerrainDrawableUBO& drawable = drawableVector[uboIndex];

    // Same elevation displacement as the terrain shader (mtl/terrain.hpp),
    // rendering only depth for the symbol occlusion pass
    const float2 pos = float2(vertx.pos);
    const float2 uv = pos / 8192.0;

    const float2 dem_uv = uv * drawable.dem_coords.x + drawable.dem_coords.yz;
    float4 demSample = demTexture.sample(demSampler, dem_uv) * 255.0;
    demSample.a = -1.0;
    const float elevation = dot(demSample, props.unpack) * props.exaggeration;

    return {
        .position = drawable.matrix * float4(pos.x, pos.y, elevation, 1.0),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]]) {
    // Pack the fragment depth into RGBA8, matching the maplibre-gl-js
    // terrain_depth shader; unpacked by unpack_depth() for calculate_visibility()
    const float depth = in.position.z;
    const float4 bit_shift = float4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
    const float4 bit_mask = float4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
    float4 res = fract(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    return half4(res);
}
)";
};

} // namespace shaders
} // namespace mbgl
