#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/webgpu/backend_impl.hpp>

namespace mbgl {
namespace webgpu {

class Context;

class IndexBufferResource : public gfx::IndexBufferResource {
public:
    IndexBufferResource(Context& context,
                       const void* data,
                       std::size_t size,
                       gfx::BufferUsageType usage,
                       bool persistent);
    ~IndexBufferResource() override;

    void update(const void* data, std::size_t size);
    
    WGPUBuffer getBuffer() const { return buffer; }
    std::size_t getSize() const { return size; }

private:
    Context& context;
    WGPUBuffer buffer = nullptr;
    std::size_t size;
    gfx::BufferUsageType usage;
    bool persistent;
};

} // namespace webgpu
} // namespace mbgl