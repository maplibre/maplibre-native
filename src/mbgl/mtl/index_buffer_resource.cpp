#include <mbgl/mtl/index_buffer_resource.hpp>

#include <mbgl/mtl/context.hpp>

namespace mbgl {
namespace mtl {

IndexBufferResource::IndexBufferResource(BufferResource&& ptr)
    : buffer(std::move(ptr)) {
    if (buffer) {
        buffer.getContext().renderingStats().numIndexBuffers++;
        buffer.getContext().renderingStats().memIndexBuffers += buffer.getSizeInBytes();
    }
}

IndexBufferResource::~IndexBufferResource() {
    if (buffer) {
        buffer.getContext().renderingStats().numIndexBuffers--;
        buffer.getContext().renderingStats().memIndexBuffers -= buffer.getSizeInBytes();
    }
}

} // namespace mtl
} // namespace mbgl
