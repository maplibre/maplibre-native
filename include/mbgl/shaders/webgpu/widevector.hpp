#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "WideVectorShader";
    static const std::array<AttributeInfo, 3> attributes;
    static const std::array<AttributeInfo, 4> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;
};

} // namespace shaders
} // namespace mbgl