// Generated code, do not modify this file!
#pragma once
#include <mln/shaders/shader_source.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::GlobeDepthShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "GlobeDepthShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;

void main() {
    gl_Position = projectTileFor3D(a_pos, 0.0);
}
)";
    static constexpr const char* fragment = R"(void main() {
    fragColor = vec4(0.0);
}
)";
};

} // namespace shaders
} // namespace mln
