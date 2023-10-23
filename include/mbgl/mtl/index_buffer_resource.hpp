#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/mtl/buffer_resource.hpp>

namespace mbgl {
namespace mtl {

class IndexBufferResource : public gfx::IndexBufferResource {
public:
    IndexBufferResource() = default;
    IndexBufferResource(BufferResource&&);
    IndexBufferResource(IndexBufferResource&& other)
        : buffer(std::move(other.buffer)) {}
    ~IndexBufferResource() override;

    std::size_t getSizeInBytes() const { return buffer.getSizeInBytes(); }
    void* contents() const { return buffer.contents(); }

    BufferResource& get() { return buffer; }
    const BufferResource& get() const { return buffer; }

protected:
    BufferResource buffer;
};

} // namespace mtl
} // namespace mbgl
