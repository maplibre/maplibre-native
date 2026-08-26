#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/vulkan/shader_program.hpp>
#include <mln/util/mat4.hpp>

namespace mln {
namespace shaders {

struct ClipUBO {
    mat4 matrix;
    std::uint32_t stencil_ref;
};

template <>
struct ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "ClippingMaskProgram";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = "";
    static constexpr auto vertex = R"(
        layout(location = 0) in ivec2 position;

#ifdef PROJECTION_GLOBE
        layout(push_constant) uniform constants {
            mat4 matrix;
            vec4 tile_mercator_coords;
            vec4 clipping_plane;
        } constant;

        void main() {
            ProjectionUBO projection;
            projection.matrix = constant.matrix;
            projection.fallback_matrix = mat4(0.0);
            projection.tile_mercator_coords = constant.tile_mercator_coords;
            projection.clipping_plane = constant.clipping_plane;
            projection.projection_transition = 1.0;
            projection.depth_offset = 0.0;
            projection.pad1 = 0.0;
            projection.pad2 = 0.0;
            gl_Position = projectTile(vec2(position), vec2(position), projection);
            gl_Position.y *= -1.0;
        }
#else
        layout(push_constant) uniform constants {
            mat4 matrix;
        } constant;

        void main() {
            gl_Position = constant.matrix * vec4(position, 0.0, 1.0);
            gl_Position.y *= -1.0;
        }
#endif
    )";

    static constexpr auto fragment = R"(
        layout(location = 0) out vec4 outColor;

        void main() {
            outColor = vec4(0.0f);
        }
    )";
};

} // namespace shaders
} // namespace mln
