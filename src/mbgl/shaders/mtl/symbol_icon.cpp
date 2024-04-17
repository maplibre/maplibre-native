#include <mbgl/shaders/mtl/symbol_icon.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, true, sizeof(SymbolDynamicUBO), idSymbolDynamicUBO},
    UniformBlockInfo{true, true, sizeof(SymbolDrawableUBO), idSymbolDrawableUBO},
    UniformBlockInfo{true, true, sizeof(SymbolTilePropsUBO), idSymbolTilePropsUBO},
    UniformBlockInfo{true, false, sizeof(SymbolInterpolateUBO), idSymbolInterpolateUBO},
    UniformBlockInfo{true, true, sizeof(SymbolEvaluatedPropsUBO), idSymbolEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 6> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::attributes = {
    // always attributes
    AttributeInfo{symbolUBOCount + 0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{symbolUBOCount + 1, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
    AttributeInfo{symbolUBOCount + 2, gfx::AttributeDataType::Short4, idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{symbolUBOCount + 3, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{symbolUBOCount + 4, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},

    // sometimes uniforms
    AttributeInfo{symbolUBOCount + 5, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

} // namespace shaders
} // namespace mbgl
