#include <mbgl/shaders/mtl/widevector.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using WideVectorShaderSource = ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> WideVectorShaderSource::attributes = {
    AttributeInfo{wideVectorUBOCount + 0, gfx::AttributeDataType::Float3, idWideVectorScreenPos},
    AttributeInfo{wideVectorUBOCount + 1, gfx::AttributeDataType::Float4, idWideVectorColor},
    AttributeInfo{wideVectorUBOCount + 2, gfx::AttributeDataType::Int, idWideVectorIndex},
};
const std::array<AttributeInfo, 4> WideVectorShaderSource::instanceAttributes = {
    AttributeInfo{wideVectorUBOCount + 3, gfx::AttributeDataType::Float3, idWideVectorInstanceCenter},
    AttributeInfo{wideVectorUBOCount + 3, gfx::AttributeDataType::Float4, idWideVectorInstanceColor},
    AttributeInfo{wideVectorUBOCount + 3, gfx::AttributeDataType::Int, idWideVectorInstancePrevious},
    AttributeInfo{wideVectorUBOCount + 3, gfx::AttributeDataType::Int, idWideVectorInstanceNext},
};
const std::array<TextureInfo, 0> WideVectorShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
