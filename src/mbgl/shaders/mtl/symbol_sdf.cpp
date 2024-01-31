#include <mbgl/shaders/mtl/symbol_sdf.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 10> ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Metal>::attributes =
    {
        // always attributes
        AttributeInfo{0, gfx::AttributeDataType::Short4, "a_pos_offset"},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, "a_data"},
        AttributeInfo{2, gfx::AttributeDataType::Short4, "a_pixeloffset"},
        AttributeInfo{3, gfx::AttributeDataType::Float3, "a_projected_pos"},
        AttributeInfo{4, gfx::AttributeDataType::Float, "a_fade_opacity"},

        // sometimes uniforms
        AttributeInfo{5, gfx::AttributeDataType::Float4, "a_fill_color"},
        AttributeInfo{6, gfx::AttributeDataType::Float4, "a_halo_color"},
        AttributeInfo{7, gfx::AttributeDataType::Float, "a_opacity"},
        AttributeInfo{8, gfx::AttributeDataType::Float, "a_halo_width"},
        AttributeInfo{9, gfx::AttributeDataType::Float, "a_halo_blur"},
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
    TextureInfo{0, "u_texture"},
};

} // namespace shaders
} // namespace mbgl
