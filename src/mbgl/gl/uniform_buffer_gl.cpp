#include <mbgl/gl/context.hpp>
#include <mbgl/gl/uniform_buffer_gl.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/util/compression.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#include <atomic>
#include <vector>
#include <cassert>

namespace mbgl {
namespace gl {

using namespace platform;

namespace {

// Currently unique IDs for constant buffers are only used when Tracy profiling is enabled
#ifdef MLN_TRACY_ENABLE
int64_t generateDebugId() noexcept {
    static std::atomic_int64_t counter(0);
    return ++counter;
}
#endif

} // namespace

UniformBufferGL::UniformBufferGL(Context& context_, const void* data_, std::size_t size_, IBufferAllocator& allocator_)
    : UniformBuffer(size_),
      context(context_),
#ifdef MLN_TRACY_ENABLE
      uniqueDebugId(generateDebugId()),
#endif
      managedBuffer(allocator_, this) {

    context.renderingStats().numUniformBuffers++;
    context.renderingStats().memUniformBuffers += size;

    context.renderingStats().totalBuffers++;
    context.renderingStats().numBuffers++;
    context.renderingStats().memBuffers += size;

#ifdef __EMSCRIPTEN__
    constexpr bool forceDisableManagedAllocation{true};
#else
    constexpr bool forceDisableManagedAllocation{false};
#endif

    MLN_TRACE_ALLOC_CONST_BUFFER(uniqueDebugId, size_);
    if (forceDisableManagedAllocation || size_ > managedBuffer.allocator.pageSize()) {
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
      context(rhs.context),
#ifdef MLN_TRACY_ENABLE
      uniqueDebugId(rhs.uniqueDebugId),
#endif
      isManagedAllocation(rhs.isManagedAllocation),
      localID(rhs.localID),
      managedBuffer(std::move(rhs.managedBuffer)) {
    managedBuffer.setOwner(this);
#ifdef MLN_TRACY_ENABLE
    rhs.uniqueDebugId = -1;
#endif
}

UniformBufferGL::UniformBufferGL(const UniformBufferGL& other)
    : UniformBuffer(other),
      context(other.context),
#ifdef MLN_TRACY_ENABLE
      uniqueDebugId(generateDebugId()),
#endif
      managedBuffer(other.managedBuffer.allocator, this) {
    MLN_TRACE_ALLOC_CONST_BUFFER(uniqueDebugId, other.size);
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
#ifdef MLN_TRACY_ENABLE
    assert(uniqueDebugId > 0);
#endif

    context.renderingStats().numUniformBuffers--;
    context.renderingStats().memUniformBuffers -= size;

    context.renderingStats().numBuffers--;
    context.renderingStats().memBuffers -= size;

    MLN_TRACE_FREE_CONST_BUFFER(uniqueDebugId);
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

void UniformBufferGL::update(const void* data, std::size_t dataSize) {
    assert(isManagedAllocation ? dataSize <= managedBuffer.getContents().size() : dataSize <= size);

    if (dataSize > size || (isManagedAllocation && dataSize > managedBuffer.getContents().size())) {
        Log::Error(Event::General,
                   "Mismatched size given to UBO update, expected max " + std::to_string(size) + ", got " +
                       std::to_string(dataSize));
        return;
    }

    if (std::memcmp(data, managedBuffer.getContents().data(), dataSize) == 0) {
        return;
    }

    if (isManagedAllocation) {
        managedBuffer.allocate(data, dataSize);
    } else {
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, localID));
        MBGL_CHECK_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, data));
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    }

    context.renderingStats().numUniformUpdates++;
    context.renderingStats().bufferUpdates++;
    context.renderingStats().bufferObjUpdates++;
    context.renderingStats().uniformUpdateBytes += dataSize;
    context.renderingStats().bufferUpdateBytes += dataSize;
}

void UniformBufferArrayGL::bind() const {
    MLN_TRACE_FUNC();

    for (size_t id = 0; id < allocatedSize(); id++) {
        const auto& uniformBuffer = get(id);
        if (!uniformBuffer) continue;
        GLint binding = static_cast<GLint>(id);
        const auto& uniformBufferGL = static_cast<const UniformBufferGL&>(*uniformBuffer);
        MBGL_CHECK_ERROR(glBindBufferRange(GL_UNIFORM_BUFFER,
                                           binding,
                                           uniformBufferGL.getID(),
                                           uniformBufferGL.getManagedBuffer().getBindingOffset(),
                                           uniformBufferGL.getSize()));
    }
}

void UniformBufferArrayGL::unbind() const {
    MLN_TRACE_FUNC();

    for (size_t id = 0; id < allocatedSize(); id++) {
        const auto& uniformBuffer = get(id);
        if (!uniformBuffer) continue;
        GLint binding = static_cast<GLint>(id);
        MBGL_CHECK_ERROR(glBindBufferBase(GL_UNIFORM_BUFFER, binding, 0));
    }
}

} // namespace gl
} // namespace mbgl
