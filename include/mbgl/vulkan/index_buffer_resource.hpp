#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/vulkan/buffer_resource.hpp>

namespace mbgl {
namespace vulkan {

class IndexBufferResource : public gfx::IndexBufferResource {
public:
    IndexBufferResource() noexcept = delete;
    IndexBufferResource(BufferResource&&) noexcept;
    IndexBufferResource(IndexBufferResource&& other) noexcept
        : buffer(std::move(other.buffer)) {}
    ~IndexBufferResource() noexcept override;

    std::size_t getSizeInBytes() const noexcept { return buffer.getSizeInBytes(); }
    const void* contents() const noexcept { return buffer.contents(); }

    BufferResource& get() noexcept { return buffer; }
    const BufferResource& get() const noexcept { return buffer; }

protected:
    BufferResource buffer;
};

} // namespace vulkan
} // namespace mbgl
