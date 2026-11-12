#pragma once

#include <mln/mtl/buffer_resource.hpp>
#include <mln/util/monotonic_timer.hpp>

#include <memory>

namespace mln {
namespace mtl {

class VertexBufferResource : public gfx::VertexBufferResource {
public:
    VertexBufferResource(BufferResource&&) noexcept;
    VertexBufferResource(VertexBufferResource&&) noexcept = default;
    ~VertexBufferResource() noexcept override;

    std::size_t getSizeInBytes() const noexcept { return buffer.getSizeInBytes(); }
    const void* contents() const noexcept { return buffer.contents(); }

    BufferResource& get() noexcept { return buffer; }
    const BufferResource& get() const noexcept { return buffer; }

    std::chrono::duration<double> getLastUpdated() const { return lastUpdated; }
    void setLastUpdated(std::chrono::duration<double> time) { lastUpdated = time; }

protected:
    BufferResource buffer;
    // Zero until the first update, so comparing it with the source's modification time is defined
    std::chrono::duration<double> lastUpdated{};
};

using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;

} // namespace mtl
} // namespace mln
