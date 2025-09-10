#include <mbgl/shaders/webgpu/widevector.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/widevector_ubo.hpp>

namespace mbgl {
namespace shaders {

using WideVectorShaderSource = ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> WideVectorShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idWideVectorPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idWideVectorDataVertexAttribute},
};
const std::array<TextureInfo, 0> WideVectorShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl