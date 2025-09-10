#include <mbgl/shaders/webgpu/widevector.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/widevector_ubo.hpp>

namespace mbgl {
namespace shaders {

using WideVectorShaderSource = ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> WideVectorShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idLineDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float, idLineOffsetVertexAttribute},
};

const std::array<AttributeInfo, 4> WideVectorShaderSource::instanceAttributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float, idLinePatternFromVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float, idLinePatternToVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
};

const std::array<TextureInfo, 0> WideVectorShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl