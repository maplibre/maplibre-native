#pragma once

#include <mbgl/shaders/mtl/background.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>
    : public ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "BackgroundPatternShader";
        static constexpr UniformBlockInfo uniforms[] = {
            { 1, sizeof(BackgroundLayerUBO), true, false, "BackgroundLayerUBO" },
            { 2, sizeof(BackgroundDrawableUBO), true, false, "BackgroundDrawableUBO" },
            { 3, sizeof(BackgroundPatternLayerUBO), true, false, "BackgroundPatternLayerUBO" },
        };
};

} // namespace shaders
} // namespace mbgl
