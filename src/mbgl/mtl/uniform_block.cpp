#include <mbgl/mtl/uniform_block.hpp>

#include <mbgl/mtl/uniform_buffer.hpp>

#include <cassert>

namespace mbgl {
namespace mtl {

void UniformBlock::bindBuffer(const gfx::UniformBuffer& uniformBuffer) {
    assert(size == uniformBuffer.getSize());
    //GLint binding = index;
    //const auto& uniformBufferGL = static_cast<const UniformBufferGL&>(uniformBuffer);
    //MBGL_CHECK_ERROR(glBindBufferBase(GL_UNIFORM_BUFFER, binding, uniformBufferGL.getID()));
}

void UniformBlock::unbindBuffer() {
    //GLint binding = index;
    //MBGL_CHECK_ERROR(glBindBufferBase(GL_UNIFORM_BUFFER, binding, 0));
}

} // namespace mtl
} // namespace mbgl
