#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/vulkan/shader_program.hpp>
#include <mln/shaders/vulkan/terrain.hpp>

namespace mln {
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

layout(location = 0) in ivec4 in_position; // xy = tile position, z = skirt flag (1 = skirt)

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
    const vec2 pos = vec2(in_position.xy);

    const float elevation = get_elevation(pos, dem_sampler, drawable.dem_coords, props.unpack,
                                          drawable.dem_coords.w, props.exaggeration, 1.0);

    // Skirt vertices drop below the surface by elevation_offset (gl-js u_ele_delta)
    const float ele_delta = in_position.z == 1 ? props.elevation_offset : 0.0;
    gl_Position = drawable.matrix * vec4(pos.x, pos.y, elevation - ele_delta, 1.0);
    // Must match the transform the symbol pass applies, so the depth texture
    // and the symbols it occludes share one clip space
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) out vec4 out_color;

void main() {
    // Pack the window depth into RGBA8 as four base-256 digits, each written as k / 255 so the
    // unorm8 target stores it exactly (the gl-js fract/bit_mask scheme stores k / 256 and loses
    // up to 0.002 to unorm rounding). Decoded by unpack_depth() for calculate_visibility().
    float r = min(gl_FragCoord.z, 0.99999994) * 256.0;
    const float d0 = floor(r);
    r = (r - d0) * 256.0;
    const float d1 = floor(r);
    r = (r - d1) * 256.0;
    const float d2 = floor(r);
    r = (r - d2) * 256.0;
    const float d3 = floor(r);
    out_color = vec4(d0, d1, d2, d3) / 255.0;
}
)";
};

} // namespace shaders
} // namespace mln
