#pragma once

#include <mln/shaders/terrain_layer_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>
#include <mln/shaders/mtl/terrain.hpp>

namespace mln {
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
    short4 pos [[attribute(0)]]; // xy = tile position, z = skirt flag (1 = skirt)
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
    const float2 pos = float2(vertx.pos.xy);

    const float elevation = get_elevation(pos, demTexture, demSampler, drawable.dem_coords, props.unpack,
                                          drawable.dem_coords.w, props.exaggeration, 1.0);

    // Skirt vertices drop below the surface by elevation_offset (gl-js u_ele_delta)
    const float ele_delta = (float(vertx.pos.z) == 1.0) ? props.elevation_offset : 0.0;
    return {
        .position = drawable.matrix * float4(pos.x, pos.y, elevation - ele_delta, 1.0),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]]) {
    // Pack the window depth into RGBA8 as four base-256 digits, each written as
    // k / 255 so it survives the unorm8 target exactly (the gl-js fract/bit_mask
    // scheme stores k / 256, which unorm8 rounds by up to 0.002 - at native's near
    // plane of 1 px that is 20x the depth difference the occlusion test looks
    // for). Decoded by unpack_depth() for calculate_visibility().
    float r = min(in.position.z, 0.99999994) * 256.0;
    const float d0 = floor(r);
    r = (r - d0) * 256.0;
    const float d1 = floor(r);
    r = (r - d1) * 256.0;
    const float d2 = floor(r);
    r = (r - d2) * 256.0;
    const float d3 = floor(r);
    return half4(float4(d0, d1, d2, d3) / 255.0);
}
)";
};

} // namespace shaders
} // namespace mln
