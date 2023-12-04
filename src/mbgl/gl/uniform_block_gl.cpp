#include <mbgl/gl/context.hpp>
#include <mbgl/gl/uniform_block_gl.hpp>
#include <mbgl/gl/uniform_buffer_gl.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/platform/gl_functions.hpp>

#include <cassert>

namespace mbgl {
namespace gl {

using namespace platform;

void UniformBlockGL::bindBuffer(const gfx::UniformBuffer& uniformBuffer) {
    assert(size == uniformBuffer.getSize());
    GLint binding = index;
    const auto& uniformBufferGL = static_cast<const UniformBufferGL&>(uniformBuffer);
    MBGL_CHECK_ERROR(glBindBufferRange(GL_UNIFORM_BUFFER,
                                       binding,
                                       uniformBufferGL.getID(),
                                       uniformBufferGL.getBindingOffset(),
                                       uniformBufferGL.getSize()));
}

void UniformBlockGL::unbindBuffer() {
    GLint binding = index;
    MBGL_CHECK_ERROR(glBindBufferBase(GL_UNIFORM_BUFFER, binding, 0));
}

} // namespace gl
} // namespace mbgl
