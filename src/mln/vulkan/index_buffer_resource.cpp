#include <mln/vulkan/index_buffer_resource.hpp>

#include <mln/vulkan/context.hpp>

namespace mln {
namespace vulkan {

IndexBufferResource::IndexBufferResource(BufferResource&& ptr) noexcept
    : buffer(std::move(ptr)) {
    if (buffer) {
        auto& stats = buffer.getContext().renderingStats();
        stats.numIndexBuffers++;
        stats.memIndexBuffers += buffer.getSizeInBytes();
    }
}

IndexBufferResource::~IndexBufferResource() noexcept {
    if (buffer) {
        auto& stats = buffer.getContext().renderingStats();
        stats.numIndexBuffers--;
        stats.memIndexBuffers -= buffer.getSizeInBytes();
    }
}

} // namespace vulkan
} // namespace mln
