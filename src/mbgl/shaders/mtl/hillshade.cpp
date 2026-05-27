#include <mbgl/shaders/mtl/hillshade.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using HillshadeShaderSource = ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> HillshadeShaderSource::attributes = {
    AttributeInfo{hillshadeUBOCount + 0, gfx::AttributeDataType::Short2, idHillshadePosVertexAttribute},
    AttributeInfo{hillshadeUBOCount + 1, gfx::AttributeDataType::Short2, idHillshadeTexturePosVertexAttribute},
};
const std::array<TextureInfo, 1> HillshadeShaderSource::textures = {
    TextureInfo{0, idHillshadeImageTexture},
};

} // namespace shaders
} // namespace mbgl
