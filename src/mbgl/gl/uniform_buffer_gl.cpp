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
    : UniformBuffer(size_),
      managedBuffer(allocator_, this) {
    if (size_ > managedBuffer.allocator.pageSize()) {
        // Buffer is very large, won't fit in the provided allocator
        MBGL_CHECK_ERROR(glGenBuffers(1, &localID));
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, localID));
        MBGL_CHECK_ERROR(glBufferData(GL_UNIFORM_BUFFER, size, data_, GL_DYNAMIC_DRAW));
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
        return;
    }

    isManagedAllocation = true;
    managedBuffer.allocate(data_, size_);
}

UniformBufferGL::UniformBufferGL(UniformBufferGL&& rhs) noexcept
    : UniformBuffer(rhs.size),
      isManagedAllocation(rhs.isManagedAllocation),
      localID(rhs.localID),
      managedBuffer(std::move(rhs.managedBuffer)) {
    managedBuffer.setOwner(this);
}

UniformBufferGL::UniformBufferGL(const UniformBufferGL& other)
    : UniformBuffer(other),
      managedBuffer(other.managedBuffer.allocator, this) {
    managedBuffer.setOwner(this);
    if (other.isManagedAllocation) {
        managedBuffer.allocate(other.managedBuffer.getContents().data(), other.size);
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
        return managedBuffer.getBufferID();
    } else {
        return localID;
    }
}

void UniformBufferGL::update(const void* data_, std::size_t size_) {
    assert(size == size_);
    assert(managedBuffer.getContents().size() == size_);

    if (size != size_ || managedBuffer.getContents().size() != size_) {
        Log::Error(
            Event::General,
            "Mismatched size given to UBO update, expected " + std::to_string(size) + ", got " + std::to_string(size_));
        return;
    }

    if (std::memcmp(data_, managedBuffer.getContents().data(), managedBuffer.getContents().size()) == 0) {
        return;
    }

    if (isManagedAllocation) {
        managedBuffer.allocate(data_, size);
    } else {
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, localID));
        MBGL_CHECK_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, size_, data_));
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    }
}

} // namespace gl
} // namespace mbgl
