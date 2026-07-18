#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto terrainShaderPrelude = R"(

#define idTerrainDrawableUBO        idDrawableReservedVertexOnlyUBO
#define idTerrainTilePropsUBO       idDrawableReservedFragmentOnlyUBO
#define idTerrainEvaluatedPropsUBO  layerUBOStartId

)";

template <>
struct ShaderSource<BuiltIn::TerrainShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "TerrainShader";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto prelude = terrainShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec2 in_texture_position;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct TerrainDrawableUBO {
    mat4 matrix;
    vec4 dem_coords;
};

layout(std140, set = LAYER_SET_INDEX, binding = idTerrainDrawableUBO) readonly buffer TerrainDrawableUBOVector {
    TerrainDrawableUBO drawable_ubo[];
} drawableVector;

layout(set = LAYER_SET_INDEX, binding = idTerrainEvaluatedPropsUBO) uniform TerrainEvaluatedPropsUBO {
    vec4 unpack;
    float exaggeration;
    float elevation_offset;
    float pad1;
    float pad2;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D dem_sampler;

layout(location = 0) out vec2 frag_uv;
layout(location = 1) out float frag_elevation;

void main() {
    const TerrainDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

    // The mesh was generated with coordinates from 0 to EXTENT (8192)
    const vec2 pos = vec2(in_position);
    frag_uv = pos / 8192.0;

    // Decode the DEM and interpolate in meters via the shared helper (the packed
    // Terrain-RGB/Terrarium DEM cannot be hardware-filtered, so it is sampled
    // NEAREST and interpolated after decoding, matching maplibre-gl-js and the
    // elevated layers). Map into the bound DEM tile; an ancestor tile is bound as
    // a fallback while this tile's own DEM loads. dem_coords.w = DEM dimension.
    frag_elevation = get_elevation(pos, dem_sampler, drawable.dem_coords, props.unpack,
                                   drawable.dem_coords.w, props.exaggeration, 1.0);

    gl_Position = drawable.matrix * vec4(pos.x, pos.y, frag_elevation, 1.0);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_uv;
layout(location = 1) in float frag_elevation;
layout(location = 0) out vec4 out_color;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 1) uniform sampler2D map_sampler;

void main() {
    // Sample the map texture (render-to-texture output) for the surface color
    // Note: Y-coordinate is flipped (1.0 - y) to match OpenGL convention
    out_color = texture(map_sampler, vec2(frag_uv.x, 1.0 - frag_uv.y));

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
