#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>
#include <mbgl/util/mat4.hpp>

namespace mbgl {
namespace shaders {

struct ClipUBO {
    matf4 matrix;
    std::uint32_t stencil_ref;
};

template <>
struct ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "ClippingMaskProgram";

    static constexpr std::array<UniformBlockInfo, 0> uniforms{};
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(
        layout(location = 0) in ivec2 position;

        layout(push_constant) uniform constants {
            mat4 matrix;
        } constant;

        void main() {
            gl_Position = constant.matrix * vec4(position, 0.0, 1.0);
            gl_Position.y *= -1.0;
        }
    )";

    static constexpr auto fragment = R"(
        layout(location = 0) out vec4 outColor;

        void main() {
            outColor = vec4(0.0f);
        }
    )";
};

} // namespace shaders
} // namespace mbgl
