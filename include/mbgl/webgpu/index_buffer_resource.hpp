#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/webgpu/buffer_resource.hpp>

namespace mbgl {
namespace webgpu {

class IndexBufferResource : public gfx::IndexBufferResource {
public:
    IndexBufferResource(BufferResource&&) noexcept;
    IndexBufferResource(IndexBufferResource&& other) noexcept
        : buffer(std::move(other.buffer)) {}
    ~IndexBufferResource() noexcept override;

    std::size_t getSizeInBytes() const noexcept { return buffer.getSizeInBytes(); }
    const void* contents() const noexcept { return buffer.contents(); }

    BufferResource& get() noexcept { return buffer; }
    const BufferResource& get() const noexcept { return buffer; }

    // Compatibility with existing code
    const BufferResource& getBuffer() const { return buffer; }
    BufferResource& getBuffer() { return buffer; }

    void update(const void* data, std::size_t size) { buffer.update(data, size, 0); }

protected:
    BufferResource buffer;
};

} // namespace webgpu
} // namespace mbgl
