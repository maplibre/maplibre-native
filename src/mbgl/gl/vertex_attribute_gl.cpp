#include <mbgl/gl/vertex_attribute_gl.hpp>

#include <mbgl/gl/defines.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>

#include <cstring>
#include <sstream>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace gl {

class VertexAttributeArrayGL;

using namespace platform;

int VertexAttributeGL::getSize(GLenum glType) {
    switch (glType) {
        case GL_FLOAT:              return 4*1;
        case GL_FLOAT_VEC2:         return 4*2;
        case GL_FLOAT_VEC3:         return 4*3;
        case GL_FLOAT_VEC4:         return 4*4;
        case GL_FLOAT_MAT2:         return 4*4;
        case GL_INT:                return 4*1;
        case GL_UNSIGNED_INT:       return 4*1;
        case GL_INT_VEC2:           return 4*2;
        case GL_INT_VEC3:           return 4*3;
        case GL_INT_VEC4:           return 4*4;
        default:                    return 0;
    }
}

int VertexAttributeGL::getStride(GLenum glType) {
    switch (glType) {
        case GL_FLOAT:              return 4*1;
        case GL_FLOAT_VEC2:         return 4*2;
        case GL_FLOAT_VEC3:         return 4*4;
        case GL_FLOAT_VEC4:         return 4*4;
        case GL_FLOAT_MAT2:         return 4*4;
        case GL_INT:                return 4*1;
        case GL_UNSIGNED_INT:       return 4*1;
        case GL_INT_VEC2:           return 4*2;
        case GL_INT_VEC3:           return 4*4;
        case GL_INT_VEC4:           return 4*4;
        default:                    return 0;
    }
}

// Copy the transformed type into the buffer, returning true if it works.
template <typename T, typename R>
static bool get(const gfx::VertexAttribute::ElementType& element, uint8_t* buffer, std::function<R(T)> f) {
    if (auto* p = std::get_if<T>(&element)) {
        *reinterpret_cast<R*>(buffer) = f(*p);
        return true;
    }
    return false;
}

// Copy the value into the buffer with a simple cast, returning true if it works.
template <typename T, typename R>
static bool get(const gfx::VertexAttribute::ElementType& element, uint8_t* buffer) {
    return get<T,R>(element, buffer, [](T x)->R{ return static_cast<R>(x); });
}

// Copy a particular type from the variant into the specified buffer, returning true if it works.
template <typename T>
static bool get(const gfx::VertexAttribute::ElementType& element, uint8_t* buffer) {
    return get<T,T>(element, buffer, [](T x)->T{ return x; });
}

template <typename T, typename R> R cast(T x) { return static_cast<R>(x); }

// Copy the variant value into the buffer, converting the type if necessary and possible
bool VertexAttributeGL::get(const gfx::VertexAttribute::ElementType& element, GLenum glType, uint8_t* buffer) {
    // TODO: do we need to handle unsigned inputs?
    switch (glType) {
        case GL_FLOAT:             return gl::get<float>(element, buffer) ||
                                          gl::get<float,std::int32_t>(element, buffer);
        case GL_FLOAT_VEC2:        return gl::get<float2>(element, buffer) ||
                                          gl::get<int2,float2>(element, buffer, [](int2 x){ return float2{(float)x[0],(float)x[1]}; });
        case GL_FLOAT_VEC3:        return gl::get<float3>(element, buffer);
        case GL_FLOAT_VEC4:
        case GL_FLOAT_MAT2:        return gl::get<float4>(element, buffer);
        case GL_INT:               return gl::get<std::int32_t>(element, buffer) ||
                                          gl::get<float,       std::int32_t> (element, buffer);
        case GL_UNSIGNED_INT:      return gl::get<std::int32_t,std::uint32_t>(element, buffer) ||
                                          gl::get<float,       std::uint32_t>(element, buffer);
        case GL_INT_VEC2:          return gl::get<int2>(element, buffer) ||
                                          gl::get<float2,int2>(element, buffer, [](float2 x){ return int2{(std::int32_t)x[0],(std::int32_t)x[1]}; });
        case GL_UNSIGNED_INT_VEC2: return false;    // TODO
        case GL_INT_VEC3:          return gl::get<int3>(element, buffer);
        case GL_UNSIGNED_INT_VEC3: return false;    // TODO
        case GL_INT_VEC4:          return gl::get<int4>(element, buffer);
        case GL_UNSIGNED_INT_VEC4: return false;    // TODO
        default:                   return false;
    }
}

void VertexAttributeGL::setGLType(platform::GLenum value) {
    glType = value;
    stride = getStride(glType);
}

std::size_t VertexAttributeGL::getStride() const {
    return getStride(getGLType());
}

const std::vector<std::uint8_t>& VertexAttributeGL::getRaw(platform::GLenum type) const {
    if (dirty || rawType != type) {
        const auto count = getCount();
        //const auto size_ = getSize(type);
        const auto stride_ = getStride(type);

        rawData.resize(stride_ * count);
        std::fill(rawData.begin(), rawData.end(), 0);

        if (!rawData.empty()) {
            std::uint8_t* outPtr = &rawData[0];
            for (std::size_t i = 0; i < count; ++i) {
                if (!get(items[i], type, outPtr)) {
                    // throw?
                }
                outPtr += stride_;
            }
        }

        dirty = false;
        rawType = type;
    }
    return rawData;
}

namespace {
    template <typename T> void applyUniform(GLint, const T&);
    template <> void applyUniform(GLint, const std::int32_t&) { }
    template <> void applyUniform(GLint, const gfx::VertexAttribute::int2&) { }
    template <> void applyUniform(GLint, const gfx::VertexAttribute::int3&) { }
    template <> void applyUniform(GLint, const gfx::VertexAttribute::int4&) { }
    template <> void applyUniform(GLint, const float& ) { }
    template <> void applyUniform(GLint, const gfx::VertexAttribute::float2& ) { }
    template <> void applyUniform(GLint, const gfx::VertexAttribute::float3& ) { }
    template <> void applyUniform(GLint, const gfx::VertexAttribute::float4& ) { }
    template <> void applyUniform(GLint, const gfx::VertexAttribute::matf3& ) { }
    template <> void applyUniform(GLint location, const gfx::VertexAttribute::matf4& value) {
        MBGL_CHECK_ERROR(glUniformMatrix4fv(location, 1, GL_FALSE, &value[0]));
    }
}

struct ApplyUniform {
    GLint location;
    template <typename T> void operator()(const T& value) { applyUniform(location, value); }
};

void VertexAttributeArrayGL::applyUniforms(const gfx::ShaderProgramBase& shader) {
    const auto& glShader = static_cast<const ShaderProgramGL&>(shader);
    const auto program = glShader.getGLProgramID();

    for (auto& kv : attrs) {
        const auto& name = kv.first;
        auto& uniform = kv.second;

        if (uniform->getIndex() < 0) {
            const auto index = MBGL_CHECK_ERROR(glGetUniformLocation(program, name.c_str()));
            Log::Warning(Event::General, "Uniform '" + name + "' = " + std::to_string(index));
            uniform->setIndex(index);
        }
        std::visit(ApplyUniform { uniform->getIndex() }, uniform->get(0));
    }
}

} // namespace gfx
} // namespace mbgl

