#include <mbgl/gl/context.hpp>
#include <mbgl/gl/index_buffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>

namespace mbgl {
namespace gl {

IndexBufferResource::IndexBufferResource(UniqueBuffer&& buffer_, int byteSize_)
    : buffer(std::move(buffer_)),
      byteSize(byteSize_) {
    MLN_TRACE_ALLOC_INDEX_BUFFER(buffer.get(), byteSize);

    if (buffer) {
        auto& stats = buffer.get_deleter().context.renderingStats();
        stats.numIndexBuffers++;
        stats.memIndexBuffers += byteSize;

        stats.numBuffers++;
        stats.totalBuffers++;
        stats.memBuffers += byteSize;
    }
}

IndexBufferResource::~IndexBufferResource() noexcept {
    MLN_TRACE_FREE_INDEX_BUFFER(buffer.get());

    if (buffer) {
        auto& stats = buffer.get_deleter().context.renderingStats();
        stats.numIndexBuffers--;
        stats.memIndexBuffers -= byteSize;
        stats.memBuffers -= byteSize;

        assert(stats.memIndexBuffers >= 0);
    }
}

} // namespace gl
} // namespace mbgl
