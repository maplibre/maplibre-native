#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gl/object.hpp>
#include <mbgl/util/monotonic_timer.hpp>

namespace mbgl {
namespace gl {

class VertexBufferResource : public gfx::VertexBufferResource {
public:
    VertexBufferResource(UniqueBuffer&& buffer_, int byteSize_);
    ~VertexBufferResource() noexcept override;

    int getByteSize() const { return byteSize; }

    const UniqueBuffer& getBuffer() const { return buffer; }

    std::chrono::duration<double> getLastUpdated() const { return lastUpdated; }
    void setLastUpdated(std::chrono::duration<double> time) { lastUpdated = time; }

protected:
    UniqueBuffer buffer;
    int byteSize;
    std::chrono::duration<double> lastUpdated = util::MonotonicTimer::now();
};

} // namespace gl
} // namespace mbgl
