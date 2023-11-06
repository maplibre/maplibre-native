#include <mbgl/mtl/vertex_buffer_resource.hpp>

#include <mbgl/mtl/context.hpp>

namespace mbgl {
namespace mtl {

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

} // namespace mtl
} // namespace mbgl
