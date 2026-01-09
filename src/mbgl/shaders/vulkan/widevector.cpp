#include <mbgl/shaders/vulkan/widevector.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/widevector_ubo.hpp>

namespace mbgl {
namespace shaders {

using WideVectorShaderSource = ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 3> WideVectorShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float3, idWideVectorScreenPos},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idWideVectorColor},
    AttributeInfo{2, gfx::AttributeDataType::Int, idWideVectorIndex},
};
const std::array<AttributeInfo, 4> WideVectorShaderSource::instanceAttributes = {
    AttributeInfo{3, gfx::AttributeDataType::Float3, idWideVectorInstanceCenter},
    AttributeInfo{4, gfx::AttributeDataType::Float4, idWideVectorInstanceColor},
    AttributeInfo{5, gfx::AttributeDataType::Int, idWideVectorInstancePrevious},
    AttributeInfo{6, gfx::AttributeDataType::Int, idWideVectorInstanceNext},
};
const std::array<TextureInfo, 0> WideVectorShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
