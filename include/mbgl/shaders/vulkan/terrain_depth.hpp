#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>
#include <mbgl/shaders/vulkan/terrain.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::TerrainDepthShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "TerrainDepthShader";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    // Shares the terrain shader's UBO binding layout
    static constexpr auto prelude = terrainShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

layout(push_constant) uniform Constants {
    int ubo_index;
    layout(offset = 16) vec4 drape_tile;
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

void main() {
    const TerrainDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

    // Same elevation displacement as the terrain shader (vulkan/terrain.hpp),
    // rendering only depth for the symbol occlusion pass
    const vec2 pos = vec2(in_position);

    const float elevation = get_elevation(pos, dem_sampler, drawable.dem_coords, props.unpack,
                                          drawable.dem_coords.w, props.exaggeration, 1.0);

    gl_Position = drawable.matrix * vec4(pos.x, pos.y, elevation, 1.0);
    // Must match the transform the symbol pass applies, so the depth texture
    // and the symbols it occludes share one clip space
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) out vec4 out_color;

void main() {
    // Pack the fragment depth into RGBA8, matching the maplibre-gl-js
    // terrain_depth shader; unpacked by unpack_depth() for calculate_visibility()
    const float depth = gl_FragCoord.z;
    const vec4 bit_shift = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
    const vec4 bit_mask = vec4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
    vec4 res = fract(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    out_color = res;
}
)";
};

} // namespace shaders
} // namespace mbgl
