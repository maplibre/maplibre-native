#include <mbgl/shaders/mtl/widevector.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using WideVectorShaderSource = ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> WideVectorShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float3, wideVectorUBOCount + 0, idWideVectorScreenPos},
    AttributeInfo{1, gfx::AttributeDataType::Float4, wideVectorUBOCount + 0, idWideVectorColor},
    AttributeInfo{2, gfx::AttributeDataType::Int, wideVectorUBOCount + 0, idWideVectorIndex},
};
const std::array<AttributeInfo, 4> WideVectorShaderSource::instanceAttributes = {
    AttributeInfo{3, gfx::AttributeDataType::Float3, wideVectorUBOCount + 1, idWideVectorInstanceCenter},
    AttributeInfo{4, gfx::AttributeDataType::Float4, wideVectorUBOCount + 1, idWideVectorInstanceColor},
    AttributeInfo{5, gfx::AttributeDataType::Int, wideVectorUBOCount + 1, idWideVectorInstancePrevious},
    AttributeInfo{6, gfx::AttributeDataType::Int, wideVectorUBOCount + 1, idWideVectorInstanceNext},
};
const std::array<TextureInfo, 0> WideVectorShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
