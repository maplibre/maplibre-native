#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <cstddef>
#include <memory>

namespace mbgl {
namespace webgpu {

class Context;

class VertexBufferResource {
public:
    VertexBufferResource(Context& context, const void* data, std::size_t size, std::size_t vertexCount, std::size_t vertexSize);
    ~VertexBufferResource();

    WGPUBuffer getBuffer() const { return buffer; }
    std::size_t getSize() const { return size; }
    std::size_t getVertexCount() const { return vertexCount; }
    std::size_t getVertexSize() const { return vertexSize; }
    
    void update(const void* data, std::size_t updateSize);

private:
    Context& context;
    WGPUBuffer buffer = nullptr;
    std::size_t size;
    std::size_t vertexCount;
    std::size_t vertexSize;
};

using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;

} // namespace webgpu
} // namespace mbgl