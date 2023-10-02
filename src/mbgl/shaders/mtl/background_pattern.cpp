#include <mbgl/shaders/mtl/background_pattern.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 3>
    ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{1, sizeof(BackgroundLayerUBO), true, false, "BackgroundLayerUBO"},
        UniformBlockInfo{2, sizeof(BackgroundDrawableUBO), true, false, "BackgroundDrawableUBO"},
        UniformBlockInfo{3, sizeof(BackgroundPatternLayerUBO), true, false, "BackgroundLayerUBO"},
};

} // namespace shaders
} // namespace mbgl
