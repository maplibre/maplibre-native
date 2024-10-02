#include <mbgl/shaders/mtl/fill_extrusion.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>::uniforms =
    {
        UniformBlockInfo{true, false, sizeof(FillExtrusionDrawableUBO), idFillExtrusionDrawableUBO},
        UniformBlockInfo{true, false, sizeof(FillExtrusionInterpolateUBO), idFillExtrusionInterpolateUBO},
        UniformBlockInfo{true, false, sizeof(FillExtrusionPropsUBO), idFillExtrusionPropsUBO},
};
const std::array<AttributeInfo, 5> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{fillExtrusionDrawableUBOCount + 0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{
        fillExtrusionDrawableUBOCount + 1, gfx::AttributeDataType::Short4, idFillExtrusionNormalEdVertexAttribute},
    AttributeInfo{
        fillExtrusionDrawableUBOCount + 2, gfx::AttributeDataType::Float4, idFillExtrusionColorVertexAttribute},
    AttributeInfo{fillExtrusionDrawableUBOCount + 3, gfx::AttributeDataType::Float, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{
        fillExtrusionDrawableUBOCount + 4, gfx::AttributeDataType::Float, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
