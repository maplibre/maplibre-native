#include <mln/webgpu/index_buffer_resource.hpp>
#include <mln/webgpu/context.hpp>

namespace mln {
namespace webgpu {

IndexBufferResource::IndexBufferResource(BufferResource&& buffer_) noexcept
    : buffer(std::move(buffer_)) {
    if (buffer.getBuffer()) {
        auto& stats = buffer.getContext().renderingStats();
        stats.numIndexBuffers++;
        stats.memIndexBuffers += buffer.getSizeInBytes();
    }
}

IndexBufferResource::~IndexBufferResource() noexcept {
    if (buffer.getBuffer()) {
        auto& stats = buffer.getContext().renderingStats();
        stats.numIndexBuffers--;
        stats.memIndexBuffers -= buffer.getSizeInBytes();
    }
}

} // namespace webgpu
} // namespace mln
