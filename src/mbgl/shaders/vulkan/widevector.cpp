#include <mbgl/shaders/vulkan/widevector.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, false, sizeof(WideVectorUniformsUBO), idWideVectorUniformsUBO},
    UniformBlockInfo{true, false, sizeof(WideVectorUniformWideVecUBO), idWideVectorUniformWideVecUBO},
};
const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float3, idWideVectorScreenPos},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idWideVectorColor},
    AttributeInfo{2, gfx::AttributeDataType::Int, idWideVectorIndex}};
const std::array<AttributeInfo, 4>
    ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::Vulkan>::instanceAttributes = {
        AttributeInfo{3, gfx::AttributeDataType::Float3, idWideVectorInstanceCenter},
        AttributeInfo{4, gfx::AttributeDataType::Float4, idWideVectorInstanceColor},
        AttributeInfo{5, gfx::AttributeDataType::Int, idWideVectorInstancePrevious},
        AttributeInfo{6, gfx::AttributeDataType::Int, idWideVectorInstanceNext}};

} // namespace shaders
} // namespace mbgl
