#pragma once

#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = MLN_STRINGIZE(SymbolTextAndIconShader);
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static constexpr AttributeInfo attributes[] = {
        {0, gfx::AttributeDataType::Short4, 1, "a_pos_offset"},
        {1, gfx::AttributeDataType::UShort4, 1, "a_data"},
        {2, gfx::AttributeDataType::Short4, 1, "a_pixeloffset"},
        {3, gfx::AttributeDataType::Float3, 1, "a_projected_pos"},
        {4, gfx::AttributeDataType::Float, 1, "a_fade_opacity"},
        {5, gfx::AttributeDataType::Float, 1, "a_opacity"},
    };
    static constexpr UniformBlockInfo uniforms[] = {
        MLN_MTL_UNIFORM_BLOCK(8, true, true, SymbolDrawableUBO),
        MLN_MTL_UNIFORM_BLOCK(9, true, true, SymbolDrawablePaintUBO),
        MLN_MTL_UNIFORM_BLOCK(10, true, false, SymbolDrawableTilePropsUBO),
        MLN_MTL_UNIFORM_BLOCK(11, true, false, SymbolDrawableInterpolateUBO),
        MLN_MTL_UNIFORM_BLOCK(12, true, true, SymbolPermutationUBO),
        MLN_MTL_UNIFORM_BLOCK(13, true, false, ExpressionInputsUBO),
    };
    static constexpr TextureInfo textures[] = {
        {0, "u_image"},
    };

    static constexpr auto source = R"(
)";
};

} // namespace shaders
} // namespace mbgl
