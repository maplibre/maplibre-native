#include <mbgl/shaders/mtl/symbol_sdf.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 10> ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Metal>::attributes =
    {
        // always attributes
        AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::Short4, idSymbolPixelOffsetVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
        AttributeInfo{4, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},

        // sometimes uniforms
        AttributeInfo{5, gfx::AttributeDataType::Float4, idSymbolColorVertexAttribute},
        AttributeInfo{6, gfx::AttributeDataType::Float4, idSymbolHaloColorVertexAttribute},
        AttributeInfo{7, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
        AttributeInfo{8, gfx::AttributeDataType::Float, idSymbolHaloWidthVertexAttribute},
        AttributeInfo{9, gfx::AttributeDataType::Float, idSymbolHaloBlurVertexAttribute},
};
const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Metal>::uniforms =
    {
        UniformBlockInfo{10, true, true, sizeof(SymbolDrawableUBO), idSymbolDrawableUBO},
        UniformBlockInfo{11, true, true, sizeof(SymbolDynamicUBO), idSymbolDynamicUBO},
        UniformBlockInfo{12, true, true, sizeof(SymbolDrawablePaintUBO), idSymbolDrawablePaintUBO},
        UniformBlockInfo{13, true, true, sizeof(SymbolDrawableTilePropsUBO), idSymbolDrawableTilePropsUBO},
        UniformBlockInfo{14, true, false, sizeof(SymbolDrawableInterpolateUBO), idSymbolDrawableInterpolateUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

} // namespace shaders
} // namespace mbgl
