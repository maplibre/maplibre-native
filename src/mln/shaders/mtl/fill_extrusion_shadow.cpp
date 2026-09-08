#include <mln/shaders/mtl/fill_extrusion_shadow.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

//
// Fill extrusion shadow mask (roofs)

using FillExtrusionShadowMaskShaderSource =
    ShaderSource<BuiltIn::FillExtrusionShadowMaskShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 4> FillExtrusionShadowMaskShaderSource::attributes = {
    AttributeInfo{
        0, gfx::AttributeDataType::Short2, fillExtrusionShadowUBOCount + 0, idFillExtrusionShadowPosVertexAttribute},
    AttributeInfo{
        1, gfx::AttributeDataType::UShort2, fillExtrusionShadowUBOCount + 0, idFillExtrusionShadowDecimalsEdAttribute},

    // Data driven
    AttributeInfo{
        2, gfx::AttributeDataType::Float, fillExtrusionShadowUBOCount + 1, idFillExtrusionShadowBaseVertexAttribute},
    AttributeInfo{
        3, gfx::AttributeDataType::Float, fillExtrusionShadowUBOCount + 1, idFillExtrusionShadowHeightVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionShadowMaskShaderSource::textures = {};

//
// Fill extrusion shadow mask, instanced (walls)

using FillExtrusionShadowMaskInstancedShaderSource =
    ShaderSource<BuiltIn::FillExtrusionShadowMaskInstancedShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> FillExtrusionShadowMaskInstancedShaderSource::attributes = {
    AttributeInfo{
        0, gfx::AttributeDataType::Short2, fillExtrusionShadowUBOCount + 0, idFillExtrusionShadowPosVertexAttribute},
};
const std::array<AttributeInfo, 4> FillExtrusionShadowMaskInstancedShaderSource::instanceAttributes = {
    // The shader also reads this same buffer directly as an OutlineInstance array, so the two
    // entries below must stay adjacent and layout-identical to FillExtrusionLayoutVertex.
    AttributeInfo{
        1, gfx::AttributeDataType::Short2, fillExtrusionShadowUBOCount + 1, idFillExtrusionShadowOutlinePosAttribute},
    AttributeInfo{
        2, gfx::AttributeDataType::UShort2, fillExtrusionShadowUBOCount + 1, idFillExtrusionShadowDecimalsEdAttribute},

    // Data driven
    AttributeInfo{
        3, gfx::AttributeDataType::Float, fillExtrusionShadowUBOCount + 2, idFillExtrusionShadowBaseVertexAttribute},
    AttributeInfo{
        4, gfx::AttributeDataType::Float, fillExtrusionShadowUBOCount + 2, idFillExtrusionShadowHeightVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionShadowMaskInstancedShaderSource::textures = {};

} // namespace shaders
} // namespace mln
