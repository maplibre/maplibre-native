#pragma once

#include <string_view>

#include <mln/shaders/fill_layer_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static std::string_view prelude();
    static std::string_view source();
};

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillOutlineShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static std::string_view prelude();
    static std::string_view source();
};

template <>
struct ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillPatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static std::string_view prelude();
    static std::string_view source();
};

template <>
struct ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillOutlinePatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static std::string_view prelude();
    static std::string_view source();
};

template <>
struct ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillOutlineTriangulatedShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static std::string_view prelude();
    static std::string_view source();
};

} // namespace shaders
} // namespace mln
