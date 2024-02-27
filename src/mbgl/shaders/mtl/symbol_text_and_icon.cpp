#include <mbgl/shaders/mtl/symbol_text_and_icon.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 9>
    ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal>::attributes = {
        // always attributes
        AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},

        // sometimes uniforms
        AttributeInfo{4, gfx::AttributeDataType::Float4, idSymbolColorVertexAttribute},
        AttributeInfo{5, gfx::AttributeDataType::Float4, idSymbolHaloColorVertexAttribute},
        AttributeInfo{6, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
        AttributeInfo{7, gfx::AttributeDataType::Float, idSymbolHaloWidthVertexAttribute},
        AttributeInfo{8, gfx::AttributeDataType::Float, idSymbolHaloBlurVertexAttribute},
};
const std::array<UniformBlockInfo, 5>
    ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{9, true, true, sizeof(SymbolDrawableUBO), idSymbolDrawableUBO},
        UniformBlockInfo{10, true, true, sizeof(SymbolDynamicUBO), idSymbolDynamicUBO},
        UniformBlockInfo{11, true, true, sizeof(SymbolDrawablePaintUBO), idSymbolDrawablePaintUBO},
        UniformBlockInfo{12, true, true, sizeof(SymbolDrawableTilePropsUBO), idSymbolDrawableTilePropsUBO},
        UniformBlockInfo{13, true, false, sizeof(SymbolDrawableInterpolateUBO), idSymbolDrawableInterpolateUBO},
};
const std::array<TextureInfo, 2> ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{1, idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl
