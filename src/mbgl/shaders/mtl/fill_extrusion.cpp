#include <mbgl/shaders/mtl/fill_extrusion.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>::uniforms =
    {
        UniformBlockInfo{true, false, sizeof(FillExtrusionDrawableUBO), idFillExtrusionDrawableUBO},
        UniformBlockInfo{true, false, sizeof(FillExtrusionPropsUBO), idFillExtrusionPropsUBO},
        UniformBlockInfo{true, false, sizeof(FillExtrusionInterpolateUBO), idFillExtrusionInterpolateUBO},
};
const std::array<AttributeInfo, 5> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{fillExtrusionUBOCount + 0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 1, gfx::AttributeDataType::Short4, idFillExtrusionNormalEdVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 2, gfx::AttributeDataType::Float4, idFillExtrusionColorVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 3, gfx::AttributeDataType::Float, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 4, gfx::AttributeDataType::Float, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
