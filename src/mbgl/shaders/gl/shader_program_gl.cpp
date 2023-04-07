#include <mbgl/shaders/gl/shader_program_gl.hpp>

namespace mbgl {
namespace gl {

ShaderProgramGL::ShaderProgramGL(UniqueProgram&& glProgram_)
    : ShaderProgramBase(),
      glProgram(std::move(glProgram_)) {
}

ShaderProgramGL::ShaderProgramGL(ShaderProgramGL&& other)
    : ShaderProgramBase(std::forward<ShaderProgramBase&&>(other)),
      glProgram(std::move(other.glProgram)) {
}

std::shared_ptr<ShaderProgramGL> ShaderProgramGL::create(
        Context& context,
        std::string_view /*name*/,
        std::string_view vertexSource,
        std::string_view fragmentSource) noexcept(false) {
    
    const auto firstAttrib = "pos";

    // throws on compile error
    auto program = context.createProgram(
          context.createShader(ShaderType::Vertex, std::initializer_list<const char*>{vertexSource.data()}),
          context.createShader(ShaderType::Fragment, std::initializer_list<const char*>{fragmentSource.data()}),
                          firstAttrib);

    return std::make_shared<ShaderProgramGL>(std::move(program));
}

} // namespace gl
} // namespace mbgl
