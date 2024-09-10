#pragma once

#include <mbgl/vulkan/buffer_resource.hpp>

#include <memory>

namespace mbgl {
namespace vulkan {

class VertexBufferResource : public gfx::VertexBufferResource {
public:
    VertexBufferResource() noexcept = delete;
    VertexBufferResource(BufferResource&& buffer_) noexcept
        : buffer(std::move(buffer_)) {}
    VertexBufferResource(VertexBufferResource&& other) noexcept
        : buffer(std::move(other.buffer)) {}
    ~VertexBufferResource() noexcept override = default;

    std::size_t getSizeInBytes() const noexcept { return buffer.getSizeInBytes(); }
    const void* contents() const noexcept { return buffer.contents(); }

    BufferResource& get() noexcept { return buffer; }
    const BufferResource& get() const noexcept { return buffer; }

    std::chrono::duration<double> getLastUpdated() const { return lastUpdated; }
    void setLastUpdated(std::chrono::duration<double> time) { lastUpdated = time; }

protected:
    BufferResource buffer;
    std::chrono::duration<double> lastUpdated;
};

using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;

} // namespace vulkan
} // namespace mbgl
