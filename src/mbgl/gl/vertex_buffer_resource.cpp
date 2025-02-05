#include <mbgl/gl/context.hpp>
#include <mbgl/gl/vertex_buffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>

namespace mbgl {
namespace gl {

VertexBufferResource::VertexBufferResource(UniqueBuffer&& buffer_, int byteSize_)
    : buffer(std::move(buffer_)),
      byteSize(byteSize_) {
    MLN_TRACE_ALLOC_VERTEX_BUFFER(buffer.get(), byteSize);
}

VertexBufferResource::~VertexBufferResource() noexcept {
    MLN_TRACE_FREE_VERTEX_BUFFER(buffer.get());
    auto& stats = buffer.get_deleter().context.renderingStats();
    stats.memVertexBuffers -= byteSize;
    assert(stats.memVertexBuffers >= 0);
}

} // namespace gl
} // namespace mbgl
