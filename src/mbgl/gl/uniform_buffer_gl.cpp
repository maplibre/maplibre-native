#include <mbgl/gl/context.hpp>
#include <mbgl/gl/uniform_buffer_gl.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/util/compression.hpp>
#include <mbgl/util/logging.hpp>

#include <vector>
#include <cassert>

namespace mbgl {
namespace gl {

using namespace platform;

UniformBufferGL::UniformBufferGL(const void* data_, std::size_t size_, IBufferAllocator& allocator_)
    : UniformBuffer(size_), RelocatableBuffer(allocator_) {
    if (size_ > allocator.pageSize()) {
        // Buffer is very large, won't fit in the provided allocator
        MBGL_CHECK_ERROR(glGenBuffers(1, &localID));
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, localID));
        MBGL_CHECK_ERROR(glBufferData(GL_UNIFORM_BUFFER, size, data_, GL_DYNAMIC_DRAW));
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
        return;
    }

    isManagedAllocation = true;
    allocate(data_, size_);
}

UniformBufferGL::UniformBufferGL(const UniformBufferGL& other)
    : UniformBuffer(other),
      RelocatableBuffer(other.allocator) {
    if (other.isManagedAllocation) {
        allocate(other.getContents().data(), other.size);
    } else {
        MBGL_CHECK_ERROR(glGenBuffers(1, &localID));
        MBGL_CHECK_ERROR(glCopyBufferSubData(other.localID, localID, 0, 0, size));
        MBGL_CHECK_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, other.localID));
        MBGL_CHECK_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, localID));
        MBGL_CHECK_ERROR(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size));
    }
}

UniformBufferGL::~UniformBufferGL() {
    if (isManagedAllocation) {
        return;
    }

    if (localID) {
        MBGL_CHECK_ERROR(glDeleteBuffers(1, &localID));
        localID = 0;
    }
}

BufferID UniformBufferGL::getID() const {
    if (isManagedAllocation) {
        return getBufferID();
    } else {
        return localID;
    }
}

void UniformBufferGL::update(const void* data_, std::size_t size_) {
    assert(size == size_);
    assert(getContents().size() == size_);

    if (size != size_) {
        Log::Error(
            Event::General,
            "Mismatched size given to UBO update, expected " + std::to_string(size) + ", got " + std::to_string(size_));
        return;
    }

    if (isManagedAllocation) {
        allocate(data_, size);
    } else {
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, localID));
        MBGL_CHECK_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, size_, data_));
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    }
}

} // namespace gl
} // namespace mbgl
