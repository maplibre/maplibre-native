#include <mbgl/shaders/gl/shader_program_gl.hpp>

#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/platform/gl_functions.hpp>

#include <cstring>
#include <utility>

namespace mbgl {
namespace gl {

static gfx::AttributeDataType mapType(platform::GLenum attrType) {
    using T = gfx::AttributeDataType;
    switch (attrType) {
        case GL_FLOAT:              return T::Float;
        case GL_FLOAT_VEC2:         return T::Float2;
        case GL_FLOAT_VEC3:         return T::Float3;
        case GL_FLOAT_VEC4:         return T::Float4;
        case GL_FLOAT_MAT2:         return T::Float4;   // does that work ?
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:         return T::Float4;
        case GL_INT:                return T::Int;
        case GL_INT_VEC2:           return T::Int2;
        case GL_INT_VEC3:           return T::Int3;
        case GL_INT_VEC4:           return T::Int4;
        case GL_UNSIGNED_INT:       return T::UInt;
        // ES3 stuff that isn't defined yet:
        //case GL_FLOAT_MAT2x3:
        //case GL_FLOAT_MAT2x4:
        //case GL_FLOAT_MAT3x2:
        //case GL_FLOAT_MAT3x4:
        //case GL_FLOAT_MAT4x2:
        //case GL_FLOAT_MAT4x3:       return T::Invalid;
        //case GL_UNSIGNED_INT_VEC2:  return T::UInt2;
        //case GL_UNSIGNED_INT_VEC3:  return T::UInt3;
        //case GL_UNSIGNED_INT_VEC4:  return T::UInt4;
        //case GL_DOUBLE:             return T::Float;
        //case GL_DOUBLE_VEC2:        return T::Float2;
        //case GL_DOUBLE_VEC3:        return T::Float3;
        //case GL_DOUBLE_VEC4:        return T::Float4;
        //case GL_DOUBLE_MAT2:        return T::Float4;
        //case GL_DOUBLE_MAT3:
        //case GL_DOUBLE_MAT4:
        //case GL_DOUBLE_MAT2x3:
        //case GL_DOUBLE_MAT2x4:
        //case GL_DOUBLE_MAT3x2:
        //case GL_DOUBLE_MAT3x4:
        //case GL_DOUBLE_MAT4x2:
        //case GL_DOUBLE_MAT4x3:
        default:                    return T::Invalid;
    }
}

// No `AttributeDataType` for 4x4, so we have to account for it separately...
static int mapCount(const platform::GLenum attrType) {
    switch (attrType) {
        case GL_FLOAT_MAT3:         return 3;
        case GL_FLOAT_MAT4:         return 4;
        default:                    return 1;
    }
}

ShaderProgramGL::ShaderProgramGL(UniqueProgram&& glProgram_)
    : ShaderProgramBase(),
      glProgram(std::move(glProgram_)) {
}

ShaderProgramGL::ShaderProgramGL(UniqueProgram&& program,
                                 VertexAttributeArrayGL&& uniforms_,
                                 VertexAttributeArrayGL&& attributes_)
    : ShaderProgramBase(),
      glProgram(std::move(program)),
      uniforms(std::move(uniforms_)),
      vertexAttributes(std::move(attributes_)) {
}

ShaderProgramGL::ShaderProgramGL(ShaderProgramGL&& other)
    : ShaderProgramBase(std::forward<ShaderProgramBase&&>(other)),
      glProgram(std::move(other.glProgram)),
      uniforms(std::move(other.uniforms)),
      vertexAttributes(std::move(other.vertexAttributes)) {
}

using namespace platform;

static void addAttr(VertexAttributeArrayGL& attrs, const char* name, GLint index,
                    GLsizei length, GLint count, GLenum glType) {
    const auto elementType = mapType(glType);
    const auto elementCount = mapCount(glType); // number of `elementType`, hopefully temporary
    if (elementType != gfx::AttributeDataType::Invalid && length > 0) {
        if (auto newAttr = attrs.add(name, index, elementType, elementCount, count)) {
            auto* glAttr = static_cast<VertexAttributeGL*>(newAttr);
            glAttr->setGLType(glType);
        }
    }
}

std::shared_ptr<ShaderProgramGL> ShaderProgramGL::create(
        Context& context,
        std::string_view /*name*/,
        std::string_view vertexSource,
        std::string_view fragmentSource) noexcept(false) {
    
    const auto firstAttrib = "a_pos";

    // throws on compile error
    auto vertProg = context.createShader(ShaderType::Vertex, std::initializer_list<const char*>{vertexSource.data()});
    auto fragProg = context.createShader(ShaderType::Fragment, std::initializer_list<const char*>{fragmentSource.data()});
    auto program = context.createProgram(vertProg, fragProg, firstAttrib);

    // GLES3.1
    //GLint numAttribs;
    //glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);

    VertexAttributeArrayGL uniforms;

    GLint count = 0;
    GLint maxLength = 0;
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count));
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength));

    auto name = std::vector<GLchar>(maxLength);
    for (GLint index = 0; index < count; ++index) {
        GLsizei length = 0;
        GLint size = 0;
        GLenum glType = 0;
        MBGL_CHECK_ERROR(glGetActiveUniform(program, index, maxLength, &length, &size, &glType, name.data()));
        const GLint location = MBGL_CHECK_ERROR(glGetUniformLocation(program, name.data()));
        addAttr(uniforms, name.data(), location, length, size, glType);
    }

    VertexAttributeArrayGL attrs;

    count = 0;
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count));
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength));
    name.resize(maxLength);
    for (GLint index = 0; index < count; ++index) {
        GLsizei length = 0; // "number of characters actually written in name (excluding the null terminator)"
        GLint size = 0;     // "size of the attribute variable, in units of the type returned in type"
        GLenum glType = 0;
        MBGL_CHECK_ERROR(glGetActiveAttrib(program, index, maxLength, &length, &size, &glType, name.data()));
        if (!strncmp(name.data(), "gl_", 3)) { // Is there a better way to detect built-in attributes?
            continue;
        }
        const GLint location = MBGL_CHECK_ERROR(glGetAttribLocation(program, name.data()));
        addAttr(attrs, name.data(), location, length, size, glType);
    }

    return std::make_shared<ShaderProgramGL>(std::move(program), std::move(uniforms), std::move(attrs));
}

} // namespace gl
} // namespace mbgl
