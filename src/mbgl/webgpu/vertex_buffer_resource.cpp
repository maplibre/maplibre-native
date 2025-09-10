#include <mbgl/webgpu/vertex_buffer_resource.hpp>
#include <cstring>
#include <cstdlib>

namespace mbgl {
namespace webgpu {

VertexBufferResource::VertexBufferResource(const void* data, std::size_t size)
    : size_(size) {
    // TODO: Create actual WebGPU buffer
    // For now, just store the data
    if (data && size > 0) {
        buffer = malloc(size);
        if (buffer) {
            std::memcpy(buffer, data, size);
        }
    }
}

VertexBufferResource::~VertexBufferResource() {
    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

} // namespace webgpu
} // namespace mbgl