#pragma once

#include <string_view>

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

struct alignas(16) ClipUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ std::uint32_t stencil_ref;
    /* 68 */ float pad1;
    /* 72 */ float pad2;
    /* 76 */ float pad3;
    /* 80 */
};
static_assert(sizeof(ClipUBO) == 5 * 16);

template <>
struct ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal> {
    static constexpr auto name = "ClippingMaskProgram";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static std::string_view prelude();
    static std::string_view source();
};

} // namespace shaders
} // namespace mln
