#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gl/buffer_resource.hpp>
#include <mbgl/util/monotonic_timer.hpp>

namespace mbgl {
namespace gl {

class VertexBufferResource : public BufferResource, public gfx::VertexBufferResource {
public:
    using BufferResource::AsyncAllocCallback;
    using BufferResource::AsyncUpdateCallback;

    // Create a vertex buffer resource that takes ownership of buffer_
    VertexBufferResource(UniqueBuffer&& buffer_, int byteSize_);

    // Create a non-owning vertex buffer resource
    VertexBufferResource(AsyncAllocCallback alloc, AsyncUpdateCallback update, int byteSize_);

    ~VertexBufferResource() noexcept override;

    std::chrono::duration<double> getLastUpdated() const { return lastUpdated; }
    void setLastUpdated(std::chrono::duration<double> time) { lastUpdated = time; }

private:
    std::chrono::duration<double> lastUpdated = util::MonotonicTimer::now();
};

} // namespace gl
} // namespace mbgl
