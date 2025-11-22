#include <mbgl/shaders/mtl/color_relief.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{colorReliefUBOCount + 0, gfx::AttributeDataType::Short2, "pos"},
    AttributeInfo{colorReliefUBOCount + 1, gfx::AttributeDataType::Short2, "texture_pos"},
};

const std::array<TextureInfo, 3> ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "image"},
    TextureInfo{1, "elevationStops"},
    TextureInfo{2, "colorStops"},
};

} // namespace shaders
} // namespace mbgl
