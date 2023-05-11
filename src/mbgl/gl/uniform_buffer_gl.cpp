
#include <mbgl/gl/uniform_buffer_gl.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/platform/gl_functions.hpp>

namespace mbgl {
namespace gl {

using namespace platform;

UniformBufferGL::UniformBufferGL(const void* data, std::size_t size_)
    : UniformBuffer(size_) {
    MBGL_CHECK_ERROR(glGenBuffers(1, &id));
    MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, id));
    MBGL_CHECK_ERROR(glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW));
    MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

} // namespace gl
} // namespace mbgl
