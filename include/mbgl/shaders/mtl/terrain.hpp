#pragma once

#include <mbgl/shaders/terrain_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto terrainShaderPrelude = R"(

enum {
    idTerrainDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idTerrainTilePropsUBO = idDrawableReservedFragmentOnlyUBO,
    idTerrainEvaluatedPropsUBO = drawableReservedUBOCount,
    terrainUBOCount
};

struct alignas(16) TerrainDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float4 dem_coords;
    /* 80 */
};
static_assert(sizeof(TerrainDrawableUBO) == 5 * 16, "wrong size");

struct alignas(16) TerrainTilePropsUBO {
    /*  0 */ float2 dem_tl;
    /*  8 */ float dem_scale;
    /* 12 */ float pad1;
    /* 16 */
};
static_assert(sizeof(TerrainTilePropsUBO) == 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) TerrainEvaluatedPropsUBO {
    /*  0 */ float4 unpack; // DEM unpack vector for the source's encoding
    /* 16 */ float exaggeration;
    /* 20 */ float elevation_offset;
    /* 24 */ float pad1;
    /* 28 */ float pad2;
    /* 32 */
};
static_assert(sizeof(TerrainEvaluatedPropsUBO) == 32, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::TerrainShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "TerrainShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto prelude = terrainShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(0)]];
    short2 texture_pos [[attribute(1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 uv;
    float elevation;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const TerrainDrawableUBO* drawableVector [[buffer(idTerrainDrawableUBO)]],
                                device const TerrainEvaluatedPropsUBO& props [[buffer(idTerrainEvaluatedPropsUBO)]],
                                texture2d<float, access::sample> demTexture [[texture(0)]],
                                sampler demSampler [[sampler(0)]]) {

    device const TerrainDrawableUBO& drawable = drawableVector[uboIndex];

    // Convert vertex position to normalized texture coordinates [0, 1]
    // The mesh was generated with coordinates from 0 to EXTENT (8192)
    float2 pos = float2(vertx.pos);
    float2 uv = pos / 8192.0;

    // Sample the DEM texture and decode elevation in meters using the source's
    // unpack vector, matching hillshade/color-relief (supports Mapbox Terrain-RGB
    // and Terrarium encodings)
    // Map into the bound DEM tile; an ancestor tile is bound as a fallback
    // while this tile's own DEM is still loading
    const float2 dem_uv = uv * drawable.dem_coords.x + drawable.dem_coords.yz;
    float4 demSample = demTexture.sample(demSampler, dem_uv) * 255.0;
    demSample.a = -1.0;
    float elevationMeters = dot(demSample, props.unpack);

    // Apply exaggeration for visible relief (default: 1.0, can be set higher for dramatic effect)
    float elevation = elevationMeters * props.exaggeration;

    // Create 3D position with elevation as Z coordinate
    float4 position = drawable.matrix * float4(pos.x, pos.y, elevation, 1.0);

    // Pack elevation value for fragment shader visualization
    float packedValue = elevation;

    return {
        .position    = position,
        .uv          = uv,
        .elevation   = packedValue,  // Pass packed RGBA to detect any non-zero values
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const TerrainEvaluatedPropsUBO& props [[buffer(idTerrainEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> mapTexture [[texture(1)]],
                            sampler mapSampler [[sampler(1)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    // Sample the map texture (render-to-texture output) for the surface color
    // Note: Y-coordinate is flipped (1.0 - y) to match OpenGL convention
    float4 mapColor = mapTexture.sample(mapSampler, float2(in.uv.x, 1.0 - in.uv.y));

    // If map texture has valid data, use it; otherwise fall back to elevation-based coloring
    // Check if alpha is > 0 to detect valid map data
    if (mapColor.a > 0.01) {
        return half4(mapColor);
    }

    // Fallback: elevation-based color gradient for debugging
    float elevation = in.elevation;
    float normalizedElevation = clamp((elevation - 500.0) / 3500.0, 0.0, 1.0);

    float3 color;
    if (normalizedElevation < 0.33) {
        float t = normalizedElevation / 0.33;
        color = mix(float3(0.2, 0.4, 0.8), float3(0.3, 0.7, 0.3), t);
    } else if (normalizedElevation < 0.66) {
        float t = (normalizedElevation - 0.33) / 0.33;
        color = mix(float3(0.3, 0.7, 0.3), float3(0.6, 0.5, 0.3), t);
    } else {
        float t = (normalizedElevation - 0.66) / 0.34;
        color = mix(float3(0.6, 0.5, 0.3), float3(0.95, 0.95, 0.95), t);
    }

    float gridLine = step(0.98, fract(in.uv.x * 4.0)) + step(0.98, fract(in.uv.y * 4.0));
    color = mix(color, float3(1.0, 1.0, 1.0), gridLine * 0.5);

    return half4(half3(color), 1.0);
}
)";
};

} // namespace shaders
} // namespace mbgl
