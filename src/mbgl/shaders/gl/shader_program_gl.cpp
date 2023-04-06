#include <mbgl/shaders/gl/shader_program_gl.hpp>

namespace mbgl {
namespace gl {

std::shared_ptr<ShaderProgramGL> ShaderProgramGL::create(
        Context& context,
        std::string_view name,
        std::string_view vertexSource,
        std::string_view fragmentSource) noexcept(false) {
    
    // search registry for existing `name`
    // If found check for duplicate/mismatch

    const auto firstAttrib = "pos";
            
    // throws on compile error
    auto program = context.createProgram(
          context.createShader(ShaderType::Vertex, std::initializer_list<const char*>{vertexSource.data()}),
          context.createShader(ShaderType::Fragment, std::initializer_list<const char*>{fragmentSource.data()}),
                          firstAttrib);

    // add to registry with `name`
    
    //return std::shared_ptr<ShaderProgramGL>(new ShaderProgramGL(std::move(program)));
    return std::make_shared<ShaderProgramGL>(std::move(program));
}

} // namespace gl
} // namespace mbgl
