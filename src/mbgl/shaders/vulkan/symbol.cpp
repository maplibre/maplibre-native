#include <mbgl/shaders/vulkan/symbol.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Symbol icon

using SymbolIconShaderSource = ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> SymbolIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idSymbolPosAttribute},
};
const std::array<AttributeInfo, 10> SymbolIconShaderSource::instanceAttributes = {
    // always attributes
    AttributeInfo{1, gfx::AttributeDataType::UShort, idSymbolSortedInstanceAttribute},

    AttributeInfo{2, gfx::AttributeDataType::Short4, idSymbolPosScaleAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{3, gfx::AttributeDataType::Short4, idSymbolOffsetTlTrAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{4, gfx::AttributeDataType::Short4, idSymbolOffsetBlBrAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, idSymbolTextureRectAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{6, gfx::AttributeDataType::Short4, idSymbolPixelOffsetAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{7, gfx::AttributeDataType::UShort4, idSymbolSizeSdfAttribute, idSymbolInstancedDrawableUBO},

    AttributeInfo{
        8, gfx::AttributeDataType::Float3, idSymbolProjectedPosAttribute, idSymbolDynamicInstancedDrawableUBO},

    AttributeInfo{9, gfx::AttributeDataType::Float, idSymbolFadeOpacityAttribute, idSymbolOpacityInstancedDrawableUBO},

    // sometimes uniforms
    AttributeInfo{10, gfx::AttributeDataType::Float2, idSymbolOpacityAttribute, idSymbolDataInstancedDrawableUBO},
};
const std::array<TextureInfo, 1> SymbolIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

//
// Symbol sdf

using SymbolSDFShaderSource = ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> SymbolSDFShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idSymbolPosAttribute},
};
const std::array<AttributeInfo, 14> SymbolSDFShaderSource::instanceAttributes = {
    // always attributes
    AttributeInfo{1, gfx::AttributeDataType::UShort, idSymbolSortedInstanceAttribute},

    AttributeInfo{2, gfx::AttributeDataType::Short4, idSymbolPosScaleAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{3, gfx::AttributeDataType::Short4, idSymbolOffsetTlTrAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{4, gfx::AttributeDataType::Short4, idSymbolOffsetBlBrAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, idSymbolTextureRectAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{6, gfx::AttributeDataType::Short4, idSymbolPixelOffsetAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{7, gfx::AttributeDataType::UShort4, idSymbolSizeSdfAttribute, idSymbolInstancedDrawableUBO},

    AttributeInfo{
        8, gfx::AttributeDataType::Float3, idSymbolProjectedPosAttribute, idSymbolDynamicInstancedDrawableUBO},

    AttributeInfo{9, gfx::AttributeDataType::Float, idSymbolFadeOpacityAttribute, idSymbolOpacityInstancedDrawableUBO},

    // sometimes uniforms
    AttributeInfo{10, gfx::AttributeDataType::Float2, idSymbolOpacityAttribute, idSymbolDataInstancedDrawableUBO},
    AttributeInfo{11, gfx::AttributeDataType::Float4, idSymbolColorAttribute, idSymbolDataInstancedDrawableUBO},
    AttributeInfo{12, gfx::AttributeDataType::Float4, idSymbolHaloColorAttribute, idSymbolDataInstancedDrawableUBO},
    AttributeInfo{13, gfx::AttributeDataType::Float2, idSymbolHaloWidthAttribute, idSymbolDataInstancedDrawableUBO},
    AttributeInfo{14, gfx::AttributeDataType::Float2, idSymbolHaloBlurAttribute, idSymbolDataInstancedDrawableUBO},
};
const std::array<TextureInfo, 1> SymbolSDFShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

//
// Symbol icon and text

using SymbolTextAndIconShaderSource = ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> SymbolTextAndIconShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idSymbolPosAttribute},
};
const std::array<AttributeInfo, 14> SymbolTextAndIconShaderSource::instanceAttributes = {
    // always attributes
    AttributeInfo{1, gfx::AttributeDataType::UShort, idSymbolSortedInstanceAttribute},

    AttributeInfo{2, gfx::AttributeDataType::Short4, idSymbolPosScaleAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{3, gfx::AttributeDataType::Short4, idSymbolOffsetTlTrAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{4, gfx::AttributeDataType::Short4, idSymbolOffsetBlBrAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, idSymbolTextureRectAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{6, gfx::AttributeDataType::Short4, idSymbolPixelOffsetAttribute, idSymbolInstancedDrawableUBO},
    AttributeInfo{7, gfx::AttributeDataType::UShort4, idSymbolSizeSdfAttribute, idSymbolInstancedDrawableUBO},

    AttributeInfo{
        8, gfx::AttributeDataType::Float3, idSymbolProjectedPosAttribute, idSymbolDynamicInstancedDrawableUBO},

    AttributeInfo{9, gfx::AttributeDataType::Float, idSymbolFadeOpacityAttribute, idSymbolOpacityInstancedDrawableUBO},

    // sometimes uniforms
    AttributeInfo{10, gfx::AttributeDataType::Float2, idSymbolOpacityAttribute, idSymbolDataInstancedDrawableUBO},
    AttributeInfo{11, gfx::AttributeDataType::Float4, idSymbolColorAttribute, idSymbolDataInstancedDrawableUBO},
    AttributeInfo{12, gfx::AttributeDataType::Float4, idSymbolHaloColorAttribute, idSymbolDataInstancedDrawableUBO},
    AttributeInfo{13, gfx::AttributeDataType::Float2, idSymbolHaloWidthAttribute, idSymbolDataInstancedDrawableUBO},
    AttributeInfo{14, gfx::AttributeDataType::Float2, idSymbolHaloBlurAttribute, idSymbolDataInstancedDrawableUBO},
};
const std::array<TextureInfo, 2> SymbolTextAndIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{1, idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl
