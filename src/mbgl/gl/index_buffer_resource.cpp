#include <mbgl/gl/context.hpp>
#include <mbgl/gl/index_buffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>

namespace mbgl {
namespace gl {

IndexBufferResource::IndexBufferResource(UniqueBuffer&& buffer_, int byteSize_)
    : buffer(std::move(buffer_)),
      byteSize(byteSize_) {
    MLN_TRACE_ALLOC_INDEX_BUFFER(buffer.get(), byteSize);
}

IndexBufferResource::~IndexBufferResource() noexcept {
    MLN_TRACE_FREE_INDEX_BUFFER(buffer.get());
    auto& stats = buffer.get_deleter().context.renderingStats();
    stats.memIndexBuffers -= byteSize;
    assert(stats.memIndexBuffers >= 0);
}

} // namespace gl
} // namespace mbgl