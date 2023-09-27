#pragma once

#include <mbgl/shaders/mtl/background.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>
    : public ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "BackgroundPatternShader";
    static const std::array<UniformBlockInfo,3> uniforms;
};

} // namespace shaders
} // namespace mbgl
