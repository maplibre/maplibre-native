#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <cstddef>
#include <memory>

namespace mbgl {
namespace webgpu {

class Context;

class IndexBufferResource {
public:
    IndexBufferResource(Context& context, const void* data, std::size_t size, std::size_t indexCount, bool uses32BitIndices = false);
    ~IndexBufferResource();

    WGPUBuffer getBuffer() const { return buffer; }
    std::size_t getSize() const { return size; }
    std::size_t getIndexCount() const { return indexCount; }
    WGPUIndexFormat getIndexFormat() const { return indexFormat; }
    
    void update(const void* data, std::size_t updateSize);

private:
    Context& context;
    WGPUBuffer buffer = nullptr;
    std::size_t size;
    std::size_t indexCount;
    WGPUIndexFormat indexFormat;
};

using UniqueIndexBufferResource = std::unique_ptr<IndexBufferResource>;

} // namespace webgpu
} // namespace mbgl