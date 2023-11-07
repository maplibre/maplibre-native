// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>


namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "ClippingMaskProgram";


    static constexpr const char* vertexData = R"(layout (location = 0) in vec2 a_pos;

uniform mat4 u_matrix;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
)";
    static constexpr const char* fragmentData = R"(void main() {
    fragColor = vec4(1.0);
}
)";
    static std::string vertex() {
        using Ty = ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::OpenGL>;
        return Ty::vertexData;
    }
    static std::string fragment() {
        using Ty = ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::OpenGL>;
        return Ty::fragmentData;
    }
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
