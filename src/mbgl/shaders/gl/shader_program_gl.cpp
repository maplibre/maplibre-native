#include <mbgl/shaders/gl/shader_program_gl.hpp>

#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/platform/gl_functions.hpp>

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
    auto vertProg = context.createShader(ShaderType::Vertex, std::initializer_list<const char*>{vertexSource.data()});
    auto fragProg = context.createShader(ShaderType::Fragment, std::initializer_list<const char*>{fragmentSource.data()});
    auto program = context.createProgram(vertProg, fragProg, firstAttrib);

    gfx::VertexAttributeArray attrs;
    auto a = attrs.add(gfx::AttributeDataType::Float, 1);

    using namespace platform;
            
    //GLint numAttribs;
    //glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);
            
    GLint count;
    GLint maxLength;
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count));
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength));

    auto name = std::make_unique<GLchar[]>(maxLength);
    GLsizei length;
    GLint size;
    GLenum type;
    for (GLint index = 0; index < count; ++index) {
        MBGL_CHECK_ERROR(glGetActiveUniform(program, index, maxLength, &length, &size, &type, name.get()));
    }

    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count));
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength));
    for (GLint index = 0; index < count; ++index) {
        MBGL_CHECK_ERROR(glGetActiveAttrib(program, index, maxLength, &length, &size, &type, name.get()));
    }

    return std::make_shared<ShaderProgramGL>(std::move(program));
}

} // namespace gl
} // namespace mbgl
