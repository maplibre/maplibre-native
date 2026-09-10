#pragma once

#include <string_view>

#include <mln/shaders/symbol_layer_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "SymbolIconShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 10> instanceAttributes;
    static const std::array<TextureInfo, 1> textures;

    static std::string_view prelude();
    static std::string_view source();
};

template <>
struct ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "SymbolSDFShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 14> instanceAttributes;
    static const std::array<TextureInfo, 1> textures;

    static std::string_view prelude();
    static std::string_view source();
};

template <>
struct ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "SymbolTextAndIconShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 14> instanceAttributes;
    static const std::array<TextureInfo, 2> textures;

    static std::string_view prelude();
    static std::string_view source();
};

} // namespace shaders
} // namespace mln
