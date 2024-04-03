#include <mbgl/shaders/mtl/symbol_icon.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 6> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::attributes = {
    // always attributes
    AttributeInfo{6, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Short4, idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},

    // sometimes uniforms
    AttributeInfo{11, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
};
const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{1, true, true, sizeof(SymbolDrawableUBO), idSymbolDrawableUBO},
    UniformBlockInfo{2, true, true, sizeof(SymbolDynamicUBO), idSymbolDynamicUBO},
    UniformBlockInfo{3, true, true, sizeof(SymbolPaintUBO), idSymbolPaintUBO},
    UniformBlockInfo{4, true, false, sizeof(SymbolTilePropsUBO), idSymbolTilePropsUBO},
    UniformBlockInfo{5, true, false, sizeof(SymbolInterpolateUBO), idSymbolInterpolateUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

} // namespace shaders
} // namespace mbgl
