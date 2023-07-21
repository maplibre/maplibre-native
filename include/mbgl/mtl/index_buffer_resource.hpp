#pragma once

#include <mbgl/mtl/buffer_resource.hpp>

namespace mbgl {
namespace mtl {

class IndexBufferResource : public gfx::IndexBufferResource {
public:
    IndexBufferResource() = default;
    IndexBufferResource(BufferResource&& ptr)
        : buffer(std::move(ptr)) {}
    IndexBufferResource(IndexBufferResource&& other)
        : buffer(std::move(other.buffer)) {}

    std::size_t getSizeInBytes() const { return buffer.getSizeInBytes(); }
    void* contents() const { return buffer.contents(); }

    BufferResource& get() { return buffer; }
    const BufferResource& get() const { return buffer; }

protected:
    BufferResource buffer;
};

} // namespace mtl
} // namespace mbgl
