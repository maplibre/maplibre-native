#include <mbgl/shaders/vulkan/symbol.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, false, sizeof(SymbolDrawableUBO), idSymbolDrawableUBO},
    UniformBlockInfo{true, true, sizeof(SymbolTilePropsUBO), idSymbolTilePropsUBO},
    UniformBlockInfo{true, false, sizeof(SymbolInterpolateUBO), idSymbolInterpolateUBO},
    UniformBlockInfo{false, true, sizeof(SymbolEvaluatedPropsUBO), idSymbolEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 6> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short4, idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Vulkan>::uniforms =
    {
        UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, true, sizeof(SymbolDrawableUBO), idSymbolDrawableUBO},
        UniformBlockInfo{true, true, sizeof(SymbolTilePropsUBO), idSymbolTilePropsUBO},
        UniformBlockInfo{true, false, sizeof(SymbolInterpolateUBO), idSymbolInterpolateUBO},
        UniformBlockInfo{false, true, sizeof(SymbolEvaluatedPropsUBO), idSymbolEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 10> ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Vulkan>::attributes =
    {
        // always attributes
        AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::Short4, idSymbolPixelOffsetVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
        AttributeInfo{4, gfx::AttributeDataType::Float2, idSymbolFadeOpacityVertexAttribute},

        // sometimes uniforms
        AttributeInfo{5, gfx::AttributeDataType::Float4, idSymbolColorVertexAttribute},
        AttributeInfo{6, gfx::AttributeDataType::Float4, idSymbolHaloColorVertexAttribute},
        AttributeInfo{7, gfx::AttributeDataType::Float2, idSymbolOpacityVertexAttribute},
        AttributeInfo{8, gfx::AttributeDataType::Float2, idSymbolHaloWidthVertexAttribute},
        AttributeInfo{9, gfx::AttributeDataType::Float2, idSymbolHaloBlurVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

const std::array<UniformBlockInfo, 5>
    ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Vulkan>::uniforms = {
        UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, true, sizeof(SymbolDrawableUBO), idSymbolDrawableUBO},
        UniformBlockInfo{true, true, sizeof(SymbolTilePropsUBO), idSymbolTilePropsUBO},
        UniformBlockInfo{true, false, sizeof(SymbolInterpolateUBO), idSymbolInterpolateUBO},
        UniformBlockInfo{false, true, sizeof(SymbolEvaluatedPropsUBO), idSymbolEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 9>
    ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Vulkan>::attributes = {
        // always attributes
        AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::Float2, idSymbolFadeOpacityVertexAttribute},

        // sometimes uniforms
        AttributeInfo{4, gfx::AttributeDataType::Float4, idSymbolColorVertexAttribute},
        AttributeInfo{5, gfx::AttributeDataType::Float4, idSymbolHaloColorVertexAttribute},
        AttributeInfo{6, gfx::AttributeDataType::Float2, idSymbolOpacityVertexAttribute},
        AttributeInfo{7, gfx::AttributeDataType::Float2, idSymbolHaloWidthVertexAttribute},
        AttributeInfo{8, gfx::AttributeDataType::Float2, idSymbolHaloBlurVertexAttribute},
};
const std::array<TextureInfo, 2> ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Vulkan>::textures =
    {
        TextureInfo{0, idSymbolImageTexture},
        TextureInfo{1, idSymbolImageIconTexture},
};

const std::array<UniformBlockInfo, 2>
    ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Vulkan>::uniforms = {
        UniformBlockInfo{true, false, sizeof(CustomSymbolIconDrawableUBO), idCustomSymbolDrawableUBO},
        UniformBlockInfo{true, false, sizeof(CustomSymbolIconParametersUBO), idCustomSymbolParametersUBO},
};
const std::array<AttributeInfo, 2>
    ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Vulkan>::attributes = {
        // always attributes
        AttributeInfo{0, gfx::AttributeDataType::Float2, idCustomSymbolPosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::Float2, idCustomSymbolTexVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idCustomSymbolImageTexture},
};

} // namespace shaders
} // namespace mbgl
