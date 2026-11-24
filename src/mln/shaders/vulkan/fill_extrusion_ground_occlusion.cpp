#include <mln/shaders/vulkan/fill_extrusion_ground_occlusion.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

//
// Fill extrusion ground ambient occlusion, instanced (one quad per outline edge)

template <BuiltIn Shader>
using GroundOcclusionShaderSource = ShaderSource<Shader, gfx::Backend::Type::Vulkan>;

// Both draws of the ground ambient occlusion share the vertex stage. The outline is read from a
// storage buffer, like the instanced fill-extrusion walls do, to get the next vertex of each edge.
#define MLN_GROUND_OCCLUSION_ATTRIBUTES(Shader)                                                                \
    const std::array<AttributeInfo, 1> GroundOcclusionShaderSource<Shader>::attributes = {                     \
        AttributeInfo{0, gfx::AttributeDataType::Short2, idFillExtrusionGroundOcclusionPosVertexAttribute},    \
    };                                                                                                         \
    const std::array<AttributeInfo, 4> GroundOcclusionShaderSource<Shader>::instanceAttributes = {             \
        AttributeInfo{1,                                                                                       \
                      gfx::AttributeDataType::Short2,                                                          \
                      idFillExtrusionGroundOcclusionOutlinePosAttribute,                                       \
                      idFillExtrusionGroundOcclusionInstancedDrawableUBO},                                     \
        AttributeInfo{2,                                                                                       \
                      gfx::AttributeDataType::UShort2,                                                         \
                      idFillExtrusionGroundOcclusionDecimalsEdAttribute,                                       \
                      idFillExtrusionGroundOcclusionInstancedDrawableUBO}, /* Data driven */                   \
        AttributeInfo{3, gfx::AttributeDataType::Float2, idFillExtrusionGroundOcclusionBaseVertexAttribute},   \
        AttributeInfo{4, gfx::AttributeDataType::Float2, idFillExtrusionGroundOcclusionHeightVertexAttribute}, \
    };                                                                                                         \
    const std::array<TextureInfo, 0> GroundOcclusionShaderSource<Shader>::textures = {};

MLN_GROUND_OCCLUSION_ATTRIBUTES(BuiltIn::FillExtrusionGroundOcclusionShader)
MLN_GROUND_OCCLUSION_ATTRIBUTES(BuiltIn::FillExtrusionGroundOcclusionResolveShader)

#undef MLN_GROUND_OCCLUSION_ATTRIBUTES

} // namespace shaders
} // namespace mln
