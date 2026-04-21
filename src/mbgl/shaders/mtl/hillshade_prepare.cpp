#include <mbgl/shaders/mtl/hillshade_prepare.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using HillshadePrepareShaderSource = ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> HillshadePrepareShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, hillshadePrepareUBOCount + 0, idHillshadePosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short2, hillshadePrepareUBOCount + 0, idHillshadeTexturePosVertexAttribute},
};
const std::array<TextureInfo, 1> HillshadePrepareShaderSource::textures = {
    TextureInfo{0, idHillshadeImageTexture},
};

} // namespace shaders
} // namespace mbgl
