#include <mbgl/webgpu/index_buffer_resource.hpp>
#include <mbgl/webgpu/context.hpp>

namespace mbgl {
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
} // namespace mbgl
