#pragma once

#include <string_view>

#include <mln/shaders/location_indicator_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LocationIndicatorShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static std::string_view prelude();
    static std::string_view source();
};

template <>
struct ShaderSource<BuiltIn::LocationIndicatorTexturedShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LocationIndicatorTexturedShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static std::string_view prelude();
    static std::string_view source();
};

} // namespace shaders
} // namespace mln
