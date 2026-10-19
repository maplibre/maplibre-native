#include <mln/webgpu/vertex_buffer_resource.hpp>
#include <mln/webgpu/context.hpp>

namespace mln {
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
} // namespace mln
