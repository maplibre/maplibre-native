#pragma once

#include <string_view>

#include <mln/shaders/widevector_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "WideVectorShader";
    static constexpr auto vertexMainFunction = "vertexTri_wideVecPerf";
    static constexpr auto fragmentMainFunction = "fragmentTri_wideVecPerf";

    static const std::array<AttributeInfo, 3> attributes;
    static const std::array<AttributeInfo, 4> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static std::string_view prelude();
    static std::string_view source();
};

} // namespace shaders
} // namespace mln
