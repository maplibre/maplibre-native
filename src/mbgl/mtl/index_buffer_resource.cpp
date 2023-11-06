#include <mbgl/mtl/index_buffer_resource.hpp>

#include <mbgl/mtl/context.hpp>

namespace mbgl {
namespace mtl {

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

} // namespace mtl
} // namespace mbgl
