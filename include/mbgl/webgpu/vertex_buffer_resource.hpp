#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/webgpu/buffer_resource.hpp>
#include <mbgl/util/monotonic_timer.hpp>

#include <memory>

namespace mbgl {
namespace webgpu {

class Context;

class VertexBufferResource : public gfx::VertexBufferResource {
public:
    VertexBufferResource(BufferResource&&) noexcept;
    VertexBufferResource(VertexBufferResource&& other) noexcept
        : buffer(std::move(other.buffer)) {}
    ~VertexBufferResource() noexcept override;

    std::size_t getSizeInBytes() const noexcept { return buffer.getSizeInBytes(); }
    const void* contents() const noexcept { return buffer.contents(); }

    BufferResource& get() noexcept { return buffer; }
    const BufferResource& get() const noexcept { return buffer; }

    // Compatibility with existing code
    const BufferResource& getBuffer() const { return buffer; }
    BufferResource& getBuffer() { return buffer; }

    std::chrono::duration<double> getLastUpdated() const { return lastUpdated; }
    void setLastUpdated(std::chrono::duration<double> time) { lastUpdated = time; }

    void update(const void* data, std::size_t size) { buffer.update(data, size, 0); }

protected:
    BufferResource buffer;
    std::chrono::duration<double> lastUpdated;
};

using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;

} // namespace webgpu
} // namespace mbgl
