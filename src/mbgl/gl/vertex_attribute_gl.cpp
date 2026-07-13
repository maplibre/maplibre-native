#include <mbgl/gl/vertex_attribute_gl.hpp>

#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/upload_pass.hpp>
#include <mbgl/gl/vertex_buffer_resource.hpp>
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
        case GL_FLOAT:
            return 4 * 1;
        case GL_FLOAT_VEC2:
            return 4 * 2;
        case GL_FLOAT_VEC3:
            return 4 * 3;
        case GL_FLOAT_VEC4:
            return 4 * 4;
        case GL_FLOAT_MAT2:
            return 4 * 4;
        case GL_INT:
            return 4 * 1;
        case GL_UNSIGNED_INT:
            return 4 * 1;
        case GL_INT_VEC2:
            return 4 * 2;
        case GL_INT_VEC3:
            return 4 * 3;
        case GL_INT_VEC4:
            return 4 * 4;
        default:
            return 0;
    }
}

int VertexAttributeGL::getStride(GLenum glType) {
    switch (glType) {
        case GL_FLOAT:
            return 4 * 1;
        case GL_FLOAT_VEC2:
            return 4 * 2;
        case GL_FLOAT_VEC3:
            return 4 * 4;
        case GL_FLOAT_VEC4:
            return 4 * 4;
        case GL_FLOAT_MAT2:
            return 4 * 4;
        case GL_INT:
            return 4 * 1;
        case GL_UNSIGNED_INT:
            return 4 * 1;
        case GL_INT_VEC2:
            return 4 * 2;
        case GL_INT_VEC3:
            return 4 * 4;
        case GL_INT_VEC4:
            return 4 * 4;
        default:
            return 0;
    }
}

namespace detail {
// Copy the transformed type into the buffer, returning true if it works.
template <typename T, typename R, typename Func /* R(T) */>
bool get(const gfx::VertexAttribute::ElementType& element, uint8_t* buffer, Func func) {
    if (auto* p = std::get_if<T>(&element)) {
        *reinterpret_cast<R*>(buffer) = func(*p);
        return true;
    }
    return false;
}

// Copy the value into the buffer with a simple cast, returning true if it works.
template <typename T, typename R>
bool get(const gfx::VertexAttribute::ElementType& element, uint8_t* buffer) {
    return get<T, R>(element, buffer, [](T x) -> R { return static_cast<R>(x); });
}

// Copy a particular type from the variant into the specified buffer, returning true if it works.
template <typename T>
bool get(const gfx::VertexAttribute::ElementType& element, uint8_t* buffer) {
    return get<T, T>(element, buffer, [](T x) -> T { return x; });
}
} // namespace detail

template <typename T, typename R>
R cast(T x) {
    return static_cast<R>(x);
}

// Copy the variant value into the buffer, converting the type if necessary and possible
bool VertexAttributeGL::get(const gfx::VertexAttribute::ElementType& element, GLenum glType, uint8_t* buffer) {
    // TODO: do we need to handle unsigned inputs?
    switch (glType) {
        case GL_FLOAT:
            return detail::get<float>(element, buffer) || detail::get<float, std::int32_t>(element, buffer);
        case GL_FLOAT_VEC2:
            return detail::get<float2>(element, buffer) ||
                   detail::get<int2, float2>(element, buffer, [](int2 x) { return float2{(float)x[0], (float)x[1]}; });
        case GL_FLOAT_VEC3:
            return detail::get<float3>(element, buffer);
        case GL_FLOAT_VEC4:
        case GL_FLOAT_MAT2:
            return detail::get<float4>(element, buffer) ||
                   detail::get<int4, float4>(
                       element,
                       buffer,
                       [](int4 x) { return float4{(float)x[0], (float)x[1], (float)x[2], (float)x[3]}; }) ||
                   detail::get<ushort8, float4>(element, buffer, [](ushort8 x) {
                       return float4{(float)x[0], (float)x[1], (float)x[2], (float)x[3]};
                   });
        case GL_INT:
            return detail::get<std::int32_t>(element, buffer) || detail::get<float, std::int32_t>(element, buffer);
        case GL_UNSIGNED_INT:
            return detail::get<std::int32_t, std::uint32_t>(element, buffer) ||
                   detail::get<float, std::uint32_t>(element, buffer);
        case GL_INT_VEC2:
            return detail::get<int2>(element, buffer) || detail::get<float2, int2>(element, buffer, [](float2 x) {
                       return int2{(std::int32_t)x[0], (std::int32_t)x[1]};
                   });
        case GL_UNSIGNED_INT_VEC2:
            return false; // TODO
        case GL_INT_VEC3:
            return detail::get<int3>(element, buffer);
        case GL_UNSIGNED_INT_VEC3:
            return false; // TODO
        case GL_INT_VEC4:
            return detail::get<int4>(element, buffer);
        case GL_UNSIGNED_INT_VEC4:
            return false; // TODO
        default:
            return false;
    }
}

void VertexAttributeGL::setGLType(platform::GLenum value) {
    glType = value;
    stride = getStride(glType);
}

std::size_t VertexAttributeGL::getStride() const {
    return getStride(getGLType());
}

const std::vector<std::uint8_t>& VertexAttributeGL::getRaw(gfx::VertexAttribute& attr,
                                                           platform::GLenum type,
                                                           std::optional<std::chrono::duration<double>> lastUpdate) {
    const auto count = attr.getCount();
    const auto stride_ = getStride(type);
    auto& rawData = attr.getRawData();
    if (!lastUpdate || attr.isModifiedAfter(*lastUpdate) || rawData.size() != count * stride_) {
        rawData.resize(stride_ * count);

        if (!rawData.empty()) {
            std::fill(rawData.begin(), rawData.end(), 0);

            std::uint8_t* outPtr = rawData.data();
            for (std::size_t i = 0; i < count; ++i) {
                if (!get(attr.get(i), type, outPtr)) {
                    // missing type conversion
                    std::fill(outPtr, outPtr + stride_, 0);
                }
                outPtr += stride_;
            }
        }
        attr.setDirty(false);
    }
    return rawData;
}

} // namespace gl
} // namespace mbgl
