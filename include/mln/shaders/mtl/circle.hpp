#pragma once

#include <string_view>

#include <mln/shaders/circle_layer_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "CircleShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 8> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static std::string_view prelude();
    static std::string_view source();
};

} // namespace shaders
} // namespace mln
