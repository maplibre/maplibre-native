#include <mbgl/shaders/mtl/fill_extrusion_pattern.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 0>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>::attributes = {};
const std::array<UniformBlockInfo, 0>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>::uniforms = {};
const std::array<TextureInfo, 0>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
