#include <mln/shaders/webgpu/fill_extrusion_ground_occlusion.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

// The WGSL bindings are literal
static_assert(idGlobalUBOIndex == 1);
static_assert(idFillExtrusionGroundOcclusionDrawableUBO == 2);
static_assert(idFillExtrusionGroundOcclusionPropsUBO == 4);

//
// Fill extrusion ground ambient occlusion, one instance per vertex of the bucket's mesh

template <BuiltIn Shader>
using GroundOcclusionShaderSource = ShaderSource<Shader, gfx::Backend::Type::WebGPU>;

// Both draws of the ground ambient occlusion share the vertex stage
#define MLN_GROUND_OCCLUSION_ATTRIBUTES(Shader)                                                                        \
    const std::array<AttributeInfo, 1> GroundOcclusionShaderSource<Shader>::attributes = {                             \
        AttributeInfo{3, gfx::AttributeDataType::Short2, idFillExtrusionGroundOcclusionPosVertexAttribute},            \
    };                                                                                                                 \
    const std::array<AttributeInfo, 8> GroundOcclusionShaderSource<Shader>::instanceAttributes = {                     \
        AttributeInfo{4, gfx::AttributeDataType::Short2, idFillExtrusionGroundOcclusionOutlinePosAttribute},           \
        AttributeInfo{5, gfx::AttributeDataType::UShort2, idFillExtrusionGroundOcclusionDecimalsEdAttribute},          \
        AttributeInfo{6, gfx::AttributeDataType::Short2, idFillExtrusionGroundOcclusionNormal2DAttribute},             \
        AttributeInfo{7, gfx::AttributeDataType::Short2, idFillExtrusionGroundOcclusionNextPosAttribute},              \
        AttributeInfo{8, gfx::AttributeDataType::UShort2, idFillExtrusionGroundOcclusionNextDecimalsEdAttribute},      \
        AttributeInfo{                                                                                                 \
            9, gfx::AttributeDataType::Short2, idFillExtrusionGroundOcclusionNextNormal2DAttribute}, /* Data driven */ \
        AttributeInfo{10, gfx::AttributeDataType::Float2, idFillExtrusionGroundOcclusionBaseVertexAttribute},          \
        AttributeInfo{11, gfx::AttributeDataType::Float2, idFillExtrusionGroundOcclusionHeightVertexAttribute},        \
    };                                                                                                                 \
    const std::array<TextureInfo, 0> GroundOcclusionShaderSource<Shader>::textures = {};

MLN_GROUND_OCCLUSION_ATTRIBUTES(BuiltIn::FillExtrusionGroundOcclusionShader)
MLN_GROUND_OCCLUSION_ATTRIBUTES(BuiltIn::FillExtrusionGroundOcclusionResolveShader)

#undef MLN_GROUND_OCCLUSION_ATTRIBUTES

} // namespace shaders
} // namespace mln
