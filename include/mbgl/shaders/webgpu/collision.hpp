#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "CollisionBoxShader";
    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
};

template <>
struct ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "CollisionCircleShader";
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
};

} // namespace shaders
} // namespace mbgl