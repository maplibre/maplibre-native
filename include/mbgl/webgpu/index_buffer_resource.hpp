#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/webgpu/buffer_resource.hpp>

#include <cstddef>

namespace mbgl {
namespace webgpu {

class Context;

class IndexBufferResource : public gfx::IndexBufferResource {
public:
    IndexBufferResource(BufferResource&& buffer) noexcept;
    ~IndexBufferResource() noexcept override;

    const BufferResource& getBuffer() const { return buffer; }
    BufferResource& getBuffer() { return buffer; }

    std::size_t getSizeInBytes() const { return buffer.getSize(); }
    
    void update(const void* data, std::size_t size) {
        buffer.update(data, size, 0);
    }

private:
    BufferResource buffer;
};

} // namespace webgpu
} // namespace mbgl