#include <mbgl/shaders/mtl/hillshade_prepare.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using HillshadePrepareShaderSource = ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> HillshadePrepareShaderSource::attributes = {
    AttributeInfo{hillshadePrepareUBOCount + 0, gfx::AttributeDataType::Short2, idHillshadePosVertexAttribute},
    AttributeInfo{hillshadePrepareUBOCount + 1, gfx::AttributeDataType::Short2, idHillshadeTexturePosVertexAttribute},
};
const std::array<TextureInfo, 1> HillshadePrepareShaderSource::textures = {
    TextureInfo{0, idHillshadeImageTexture},
};

} // namespace shaders
} // namespace mbgl
