#include <mln/shaders/mtl/fill_extrusion_ground_occlusion.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

//
// Fill extrusion ground ambient occlusion, instanced (one quad per outline edge)

template <BuiltIn Shader>
using GroundOcclusionShaderSource = ShaderSource<Shader, gfx::Backend::Type::Metal>;

// Both draws of the ground ambient occlusion share the vertex stage
#define MLN_GROUND_OCCLUSION_ATTRIBUTES(Shader)                                                    \
    const std::array<AttributeInfo, 1> GroundOcclusionShaderSource<Shader>::attributes = {         \
        AttributeInfo{0,                                                                           \
                      gfx::AttributeDataType::Short2,                                              \
                      fillExtrusionGroundOcclusionUBOCount + 0,                                    \
                      idFillExtrusionGroundOcclusionPosVertexAttribute},                           \
    };                                                                                             \
    const std::array<AttributeInfo, 4> GroundOcclusionShaderSource<Shader>::instanceAttributes = { \
        /* The outline of the bucket, as the instanced fill-extrusion walls read it */             \
        AttributeInfo{1,                                                                           \
                      gfx::AttributeDataType::Short2,                                              \
                      fillExtrusionGroundOcclusionUBOCount + 1,                                    \
                      idFillExtrusionGroundOcclusionOutlinePosAttribute},                          \
        AttributeInfo{2,                                                                           \
                      gfx::AttributeDataType::UShort2,                                             \
                      fillExtrusionGroundOcclusionUBOCount + 1,                                    \
                      idFillExtrusionGroundOcclusionDecimalsEdAttribute}, /* Data driven */        \
        AttributeInfo{3,                                                                           \
                      gfx::AttributeDataType::Float2,                                              \
                      fillExtrusionGroundOcclusionUBOCount + 2,                                    \
                      idFillExtrusionGroundOcclusionBaseVertexAttribute},                          \
        AttributeInfo{4,                                                                           \
                      gfx::AttributeDataType::Float2,                                              \
                      fillExtrusionGroundOcclusionUBOCount + 2,                                    \
                      idFillExtrusionGroundOcclusionHeightVertexAttribute},                        \
    };                                                                                             \
    const std::array<TextureInfo, 0> GroundOcclusionShaderSource<Shader>::textures = {};

MLN_GROUND_OCCLUSION_ATTRIBUTES(BuiltIn::FillExtrusionGroundOcclusionShader)
MLN_GROUND_OCCLUSION_ATTRIBUTES(BuiltIn::FillExtrusionGroundOcclusionResolveShader)

#undef MLN_GROUND_OCCLUSION_ATTRIBUTES

} // namespace shaders
} // namespace mln
