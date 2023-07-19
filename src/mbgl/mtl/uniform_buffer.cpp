#include <mbgl/mtl/uniform_buffer.hpp>

#include <mbgl/util/logging.hpp>

#include <cassert>

namespace mbgl {
namespace mtl {

UniformBuffer::UniformBuffer(const void* data, std::size_t size_)
    : gfx::UniformBuffer(size_) {
//    MBGL_CHECK_ERROR(glGenBuffers(1, &id));
//    MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, id));
//    MBGL_CHECK_ERROR(glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW));
//    MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

UniformBuffer::~UniformBuffer() {
//    if (id) {
//        MBGL_CHECK_ERROR(glDeleteBuffers(1, &id));
//        id = 0;
//    }
}

void UniformBuffer::update(const void* data, std::size_t size_) {
    assert(size == size_);
    if (size != size_) {
        Log::Error(
            Event::General,
            "Mismatched size given to UBO update, expected " + std::to_string(size) + ", got " + std::to_string(size_));
        return;
    }

//    MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, id));
//    MBGL_CHECK_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, size_, data));
//    MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

} // namespace mtl
} // namespace mbgl
