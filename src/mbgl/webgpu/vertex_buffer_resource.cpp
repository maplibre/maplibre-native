#include <mbgl/webgpu/vertex_buffer_resource.hpp>
#include <mbgl/webgpu/context.hpp>

namespace mbgl {
namespace webgpu {

VertexBufferResource::VertexBufferResource(BufferResource&& buffer_) noexcept
    : buffer(std::move(buffer_)) {
    if (buffer.getBuffer()) {
        auto& stats = buffer.getContext().renderingStats();
        stats.numVertexBuffers++;
        stats.memVertexBuffers += buffer.getSizeInBytes();
    }
}

VertexBufferResource::~VertexBufferResource() noexcept {
    if (buffer.getBuffer()) {
        auto& stats = buffer.getContext().renderingStats();
        stats.numVertexBuffers--;
        stats.memVertexBuffers -= buffer.getSizeInBytes();
    }
}

} // namespace webgpu
} // namespace mbgl
