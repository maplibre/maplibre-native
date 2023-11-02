#pragma once

#include <mbgl/mtl/buffer_resource.hpp>

#include <memory>

namespace mbgl {
namespace mtl {

class VertexBufferResource : public gfx::VertexBufferResource {
public:
    VertexBufferResource() noexcept = default;
    VertexBufferResource(BufferResource&&) noexcept;
    VertexBufferResource(VertexBufferResource&& other) noexcept
        : buffer(std::move(other.buffer)) {}
    ~VertexBufferResource() noexcept override;

    std::size_t getSizeInBytes() const noexcept { return buffer.getSizeInBytes(); }
    const void* contents() const noexcept { return buffer.contents(); }

    BufferResource& get() noexcept { return buffer; }
    const BufferResource& get() const noexcept { return buffer; }

protected:
    BufferResource buffer;
};

using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;

} // namespace mtl
} // namespace mbgl
