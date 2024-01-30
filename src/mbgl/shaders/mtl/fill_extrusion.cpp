#include <mbgl/shaders/mtl/fill_extrusion.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 5> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Short4, "a_normal_ed"},
    AttributeInfo{2, gfx::AttributeDataType::Float4, "a_color"},
    AttributeInfo{3, gfx::AttributeDataType::Float, "a_base"},
    AttributeInfo{4, gfx::AttributeDataType::Float, "a_height"},
};
const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>::uniforms =
    {
        UniformBlockInfo{6, true, false, sizeof(FillExtrusionDrawableUBO), idFillExtrusionDrawableUBO},
        UniformBlockInfo{7, true, false, sizeof(FillExtrusionDrawablePropsUBO), idFillExtrusionDrawablePropsUBO},
        UniformBlockInfo{9, true, false, sizeof(FillExtrusionInterpolateUBO), idFillExtrusionInterpolateUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
