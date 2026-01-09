#include <mbgl/shaders/gl/shader_program_gl.hpp>

#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/shaders/shader_manifest.hpp>

#include <cstring>
#include <utility>

namespace mbgl {
namespace gl {

namespace {

gfx::AttributeDataType mapType(platform::GLenum attrType) {
    using T = gfx::AttributeDataType;
    switch (attrType) {
        case GL_FLOAT:
            return T::Float;
        case GL_FLOAT_VEC2:
            return T::Float2;
        case GL_FLOAT_VEC3:
            return T::Float3;
        case GL_FLOAT_VEC4:
            return T::Float4;
        case GL_FLOAT_MAT2:
            return T::Float4; // does that work ?
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
            return T::Float4;
        case GL_INT:
            return T::Int;
        case GL_INT_VEC2:
            return T::Int2;
        case GL_INT_VEC3:
            return T::Int3;
        case GL_INT_VEC4:
            return T::Int4;
        case GL_UNSIGNED_INT:
            return T::UInt;
        // ES3 stuff that isn't defined yet:
        // case GL_FLOAT_MAT2x3:
        // case GL_FLOAT_MAT2x4:
        // case GL_FLOAT_MAT3x2:
        // case GL_FLOAT_MAT3x4:
        // case GL_FLOAT_MAT4x2:
        // case GL_FLOAT_MAT4x3:       return T::Invalid;
        // case GL_UNSIGNED_INT_VEC2:  return T::UInt2;
        // case GL_UNSIGNED_INT_VEC3:  return T::UInt3;
        // case GL_UNSIGNED_INT_VEC4:  return T::UInt4;
        // case GL_DOUBLE:             return T::Float;
        // case GL_DOUBLE_VEC2:        return T::Float2;
        // case GL_DOUBLE_VEC3:        return T::Float3;
        // case GL_DOUBLE_VEC4:        return T::Float4;
        // case GL_DOUBLE_MAT2:        return T::Float4;
        // case GL_DOUBLE_MAT3:
        // case GL_DOUBLE_MAT4:
        // case GL_DOUBLE_MAT2x3:
        // case GL_DOUBLE_MAT2x4:
        // case GL_DOUBLE_MAT3x2:
        // case GL_DOUBLE_MAT3x4:
        // case GL_DOUBLE_MAT4x2:
        // case GL_DOUBLE_MAT4x3:
        default:
            return T::Invalid;
    }
}

using namespace platform;

void addAttr(VertexAttributeArrayGL& attrs, const size_t id, GLint index, GLsizei length, GLint count, GLenum glType) {
    const auto elementType = mapType(glType);
    if (elementType != gfx::AttributeDataType::Invalid && length > 0) {
        if (const auto& newAttr = attrs.set(id, index, elementType, count)) {
            const auto& glAttr = static_cast<VertexAttributeGL*>(newAttr.get());
            glAttr->setGLType(glType);
        }
    }
}

} // namespace

ShaderProgramGL::ShaderProgramGL(UniqueProgram&& glProgram_)
    : ShaderProgramBase(),
      glProgram(std::move(glProgram_)) {}

ShaderProgramGL::ShaderProgramGL(UniqueProgram&& program,
                                 VertexAttributeArrayGL&& attributes_,
                                 SamplerLocationArray&& samplerLocations_)
    : ShaderProgramBase(),
      glProgram(std::move(program)),
      vertexAttributes(std::move(attributes_)),
      samplerLocations(std::move(samplerLocations_)) {}

ShaderProgramGL::ShaderProgramGL(ShaderProgramGL&& other)
    : ShaderProgramBase(std::forward<ShaderProgramBase&&>(other)),
      glProgram(std::move(other.glProgram)),
      vertexAttributes(std::move(other.vertexAttributes)),
      samplerLocations(std::move(other.samplerLocations)) {}

std::optional<size_t> ShaderProgramGL::getSamplerLocation(const size_t id) const {
    return (id < samplerLocations.size()) ? samplerLocations[id] : std::nullopt;
}

std::shared_ptr<ShaderProgramGL> ShaderProgramGL::create(
    Context& context,
    const ProgramParameters& programParameters,
    const std::string_view firstAttribName,
    const std::vector<shaders::UniformBlockInfo>& uniformBlocksInfo,
    const std::vector<shaders::TextureInfo>& texturesInfo,
    const std::vector<shaders::AttributeInfo>& attributesInfo,
    const std::string& vertexSource,
    const std::string& fragmentSource,
    const std::string& additionalDefines) noexcept(false) {
    try {
        context.getObserver().onPreCompileShader(
            programParameters.getProgramType(), gfx::Backend::Type::OpenGL, additionalDefines);

        // throws on compile error
        auto vertProg = context.createShader(
            ShaderType::Vertex,
            std::initializer_list<const char*>{
                "#version 300 es\n",
                programParameters.getDefinesString().c_str(),
                additionalDefines.c_str(),
                shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::OpenGL>::vertex,
                vertexSource.c_str()});
        auto fragProg = context.createShader(
            ShaderType::Fragment,
            {"#version 300 es\n",
             programParameters.getDefinesString().c_str(),
             additionalDefines.c_str(),
             shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::OpenGL>::fragment,
             fragmentSource.c_str()});
        auto program = context.createProgram(vertProg, fragProg, firstAttribName.data());

        context.getObserver().onPostCompileShader(
            programParameters.getProgramType(), gfx::Backend::Type::OpenGL, additionalDefines);

        for (const auto& blockInfo : uniformBlocksInfo) {
            GLint index = MBGL_CHECK_ERROR(glGetUniformBlockIndex(program, blockInfo.name.data()));
            GLint size = 0;
            MBGL_CHECK_ERROR(glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_DATA_SIZE, &size));
            assert(size > 0);
            GLint binding = static_cast<GLint>(blockInfo.binding);
            MBGL_CHECK_ERROR(glUniformBlockBinding(program, index, binding));
        }

        SamplerLocationArray samplerLocations;
        for (const auto& textureInfo : texturesInfo) {
            GLint location = MBGL_CHECK_ERROR(glGetUniformLocation(program, textureInfo.name.data()));
            assert(location != -1);
            if (location != -1) {
                samplerLocations[textureInfo.id] = location;
            }
        }

        VertexAttributeArrayGL attrs;
        GLint count = 0;
        GLint maxLength = 0;
        MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count));
        MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength));
        auto name = std::vector<GLchar>(maxLength);
        for (GLint index = 0; index < count; ++index) {
            GLsizei length = 0; // "number of characters actually written in name (excluding the null terminator)"
            GLint size = 0;     // "size of the attribute variable, in units of the type returned in type"
            GLenum glType = 0;
            MBGL_CHECK_ERROR(glGetActiveAttrib(program, index, maxLength, &length, &size, &glType, name.data()));
            if (!strncmp(name.data(), "gl_", 3)) { // Is there a better way to detect built-in attributes?
                continue;
            }
            const GLint location = MBGL_CHECK_ERROR(glGetAttribLocation(program, name.data()));
            assert(attributesInfo[location].name == std::string_view(name.data()));
            addAttr(attrs, attributesInfo[location].id, location, length, size, glType);
        }

        return std::make_shared<ShaderProgramGL>(std::move(program), std::move(attrs), std::move(samplerLocations));
    } catch (const std::exception& e) {
        context.getObserver().onShaderCompileFailed(
            programParameters.getProgramType(), gfx::Backend::Type::OpenGL, additionalDefines);
        std::rethrow_exception(std::current_exception());
    }
}

} // namespace gl
} // namespace mbgl
