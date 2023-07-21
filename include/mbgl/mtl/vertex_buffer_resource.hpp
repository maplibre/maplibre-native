#pragma once

#include <mbgl/mtl/buffer_resource.hpp>

namespace mbgl {
namespace mtl {

class VertexBufferResource : public gfx::VertexBufferResource {
public:
    VertexBufferResource() = default;
    VertexBufferResource(BufferResource&& ptr) : buffer(std::move(ptr)) {}
    VertexBufferResource(VertexBufferResource&& other) : buffer(std::move(other.buffer)) {}

    std::size_t getSizeInBytes() const { return buffer.getSizeInBytes(); }
    void* contents() const { return buffer.contents(); }

    BufferResource& get() { return buffer; }
    const BufferResource& get() const { return buffer; }

protected:
    BufferResource buffer;
};

} // namespace mtl
} // namespace mbgl
