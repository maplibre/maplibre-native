#include <mbgl/mtl/vertex_buffer_resource.hpp>

#include <mbgl/mtl/context.hpp>

namespace mbgl {
namespace mtl {

VertexBufferResource::VertexBufferResource(BufferResource&& ptr)
    : buffer(std::move(ptr)) {
    if (buffer) {
        buffer.getContext().renderingStats().numVertexBuffers++;
        buffer.getContext().renderingStats().memVertexBuffers += buffer.getSizeInBytes();
    }
}

VertexBufferResource::~VertexBufferResource() {
    if (buffer) {
        buffer.getContext().renderingStats().numVertexBuffers--;
        buffer.getContext().renderingStats().memVertexBuffers -= buffer.getSizeInBytes();
    }
}

} // namespace mtl
} // namespace mbgl
