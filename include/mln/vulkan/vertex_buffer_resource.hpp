#pragma once

#include <mln/gfx/vertex_buffer.hpp>
#include <mln/vulkan/buffer_resource.hpp>

#include <mln/util/monotonic_timer.hpp>

#include <memory>

namespace mln {
namespace vulkan {

class VertexBufferResource : public gfx::VertexBufferResource {
public:
    VertexBufferResource(BufferResource&& buffer_) noexcept;
    VertexBufferResource(VertexBufferResource&& other) noexcept
        : buffer(std::move(other.buffer)),
          lastUpdated(other.lastUpdated) {}
    ~VertexBufferResource() noexcept override;

    std::size_t getSizeInBytes() const noexcept { return buffer.getSizeInBytes(); }
    const void* contents() const noexcept { return buffer.contents(); }

    BufferResource& get() noexcept { return buffer; }
    const BufferResource& get() const noexcept { return buffer; }

    std::chrono::duration<double> getLastUpdated() const { return lastUpdated; }
    void setLastUpdated(std::chrono::duration<double> time) { lastUpdated = time; }

protected:
    BufferResource buffer;
    std::chrono::duration<double> lastUpdated = util::MonotonicTimer::now();
};

using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;

} // namespace vulkan
} // namespace mln
