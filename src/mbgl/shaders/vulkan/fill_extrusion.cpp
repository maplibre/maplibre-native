#include <mbgl/shaders/vulkan/fill_extrusion.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Fill extrusion

using FillExtrusionShaderSource = ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 4> FillExtrusionShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillExtrusionColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionShaderSource::textures = {};

//
// Fill extrusion instanced

using FillExtrusionInstancedShaderSource =
    ShaderSource<BuiltIn::FillExtrusionInstancedShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> FillExtrusionInstancedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
};
const std::array<AttributeInfo, 5> FillExtrusionInstancedShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short2, idFillExtrusionOutlinePosAttribute, true},
    AttributeInfo{2, gfx::AttributeDataType::UShort2, idFillExtrusionEdDiscardAttribute, true},

    // Data driven
    AttributeInfo{3, gfx::AttributeDataType::Float4, idFillExtrusionColorVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionInstancedShaderSource::textures = {};

//
// Fill extrusion pattern

using FillExtrusionPatternShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 5> FillExtrusionPatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::UShort4, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

//
// Fill extrusion pattern instanced

using FillExtrusionPatternInstancedShaderSource =
    ShaderSource<BuiltIn::FillExtrusionPatternInstancedShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> FillExtrusionPatternInstancedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
};
const std::array<AttributeInfo, 6> FillExtrusionPatternInstancedShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short2, idFillExtrusionOutlinePosAttribute, true},
    AttributeInfo{2, gfx::AttributeDataType::UShort2, idFillExtrusionEdDiscardAttribute, true},

    // Data driven
    AttributeInfo{3, gfx::AttributeDataType::Float, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UShort4, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternInstancedShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mbgl
