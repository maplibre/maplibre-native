#include <mbgl/gl/context.hpp>
#include <mbgl/gl/vertex_buffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <cassert>

namespace mbgl {
namespace gl {

VertexBufferResource::VertexBufferResource(UniqueBuffer&& buffer_, int byteSize_)
    : BufferResource(std::move(buffer_), byteSize_) {
    MLN_TRACE_ALLOC_VERTEX_BUFFER(buffer->get(), byteSize);
}

VertexBufferResource::VertexBufferResource(AsyncAllocCallback alloc, AsyncUpdateCallback update, int byteSize_)
    : BufferResource(std::move(alloc), std::move(update)) {
    (void)byteSize_; // Unused if Tracy is not enabled
    MLN_TRACE_ALLOC_VERTEX_BUFFER(buffer->get(), byteSize_);
}

VertexBufferResource::~VertexBufferResource() noexcept {
    MLN_TRACE_FUNC();
    // Must wait before destruction in case a resource is created but never used (wait never called)
    wait();

    auto& underlyingBuffer = *buffer;
    MLN_TRACE_FREE_VERTEX_BUFFER(underlyingBuffer.get());
    auto& stats = underlyingBuffer.get_deleter().context.renderingStats();
    stats.memVertexBuffers -= byteSize;
    assert(stats.memVertexBuffers >= 0);
}

} // namespace gl
} // namespace mbgl