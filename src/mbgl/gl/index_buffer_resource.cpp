#include <mbgl/gl/context.hpp>
#include <mbgl/gl/index_buffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <cassert>

namespace mbgl {
namespace gl {

IndexBufferResource::IndexBufferResource(UniqueBuffer&& buffer_, int byteSize_)
    : BufferResource(std::move(buffer_), byteSize_) {
    MLN_TRACE_ALLOC_INDEX_BUFFER(buffer->get(), byteSize);
}

IndexBufferResource::IndexBufferResource(AsyncAllocCallback alloc, AsyncUpdateCallback update, int byteSize_)
    : BufferResource(std::move(alloc), std::move(update)) {
    (void)byteSize_; // Unused if Tracy is not enabled
    MLN_TRACE_ALLOC_INDEX_BUFFER(buffer->get(), byteSize_);
}

IndexBufferResource::~IndexBufferResource() noexcept {
    MLN_TRACE_FUNC();

    // Must wait before destruction in case a resource is created but never used (wait never called)
    wait();

    auto& underlyingBuffer = *buffer;
    MLN_TRACE_FREE_INDEX_BUFFER(underlyingBuffer.get());
    auto& stats = underlyingBuffer.get_deleter().context.renderingStats();
    stats.memIndexBuffers -= byteSize;
    assert(stats.memIndexBuffers >= 0);
}

} // namespace gl
} // namespace mbgl
