#pragma once

#include <string_view>

#include <mln/shaders/fill_extrusion_layer_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static std::string_view prelude();
    static std::string_view source();
};

template <>
struct ShaderSource<BuiltIn::FillExtrusionInstancedShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionInstancedShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 5> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static std::string_view prelude();
    static std::string_view source();
};

template <>
struct ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionPatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 6> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static std::string_view prelude();
    static std::string_view source();
};

template <>
struct ShaderSource<BuiltIn::FillExtrusionPatternInstancedShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionPatternInstancedShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 6> instanceAttributes;
    static const std::array<TextureInfo, 1> textures;

    static std::string_view prelude();
    static std::string_view source();
};

} // namespace shaders
} // namespace mln
