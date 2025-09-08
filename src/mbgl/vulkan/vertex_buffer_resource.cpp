#include <mbgl/vulkan/vertex_buffer_resource.hpp>

#include <mbgl/vulkan/context.hpp>

namespace mbgl {
namespace vulkan {

VertexBufferResource::VertexBufferResource(BufferResource&& ptr) noexcept
    : buffer(std::move(ptr)) {
    if (buffer) {
        auto& stats = buffer.getContext().renderingStats();
        stats.numVertexBuffers++;
        stats.memVertexBuffers += buffer.getSizeInBytes();
    }
}

VertexBufferResource::~VertexBufferResource() noexcept {
    if (buffer) {
        auto& stats = buffer.getContext().renderingStats();
        stats.numVertexBuffers--;
        stats.memVertexBuffers -= buffer.getSizeInBytes();
    }
}

} // namespace vulkan
} // namespace mbgl
