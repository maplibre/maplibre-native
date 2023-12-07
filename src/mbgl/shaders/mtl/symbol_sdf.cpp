#include <mbgl/shaders/mtl/symbol_sdf.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 10> ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Metal>::attributes =
    {
        // always attributes
        AttributeInfo{0, gfx::AttributeDataType::Short4, 1, "a_pos_offset"},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, 1, "a_data"},
        AttributeInfo{2, gfx::AttributeDataType::Short4, 1, "a_pixeloffset"},
        AttributeInfo{3, gfx::AttributeDataType::Float3, 1, "a_projected_pos"},
        AttributeInfo{4, gfx::AttributeDataType::Float, 1, "a_fade_opacity"},

        // sometimes uniforms
        AttributeInfo{5, gfx::AttributeDataType::Float4, 1, "a_fill_color"},
        AttributeInfo{6, gfx::AttributeDataType::Float4, 1, "a_halo_color"},
        AttributeInfo{7, gfx::AttributeDataType::Float, 1, "a_opacity"},
        AttributeInfo{8, gfx::AttributeDataType::Float, 1, "a_halo_width"},
        AttributeInfo{9, gfx::AttributeDataType::Float, 1, "a_halo_blur"},
};
const std::array<UniformBlockInfo, 6> ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Metal>::uniforms =
    {
        UniformBlockInfo{10, true, false, sizeof(MatrixUBO), "SymbolMatrixUBO"},
        UniformBlockInfo{11, true, true, sizeof(SymbolDrawableUBO), "SymbolDrawableUBO"},
        UniformBlockInfo{12, true, true, sizeof(SymbolDynamicUBO), "SymbolDynamicUBO"},
        UniformBlockInfo{13, true, true, sizeof(SymbolDrawablePaintUBO), "SymbolDrawablePaintUBO"},
        UniformBlockInfo{14, true, true, sizeof(SymbolDrawableTilePropsUBO), "SymbolDrawableTilePropsUBO"},
        UniformBlockInfo{15, true, false, sizeof(SymbolDrawableInterpolateUBO), "SymbolDrawableInterpolateUBO"},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_texture"},
};

} // namespace shaders
} // namespace mbgl
