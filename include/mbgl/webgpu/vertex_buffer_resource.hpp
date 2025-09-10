#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/webgpu/buffer_resource.hpp>

#include <cstddef>

namespace mbgl {
namespace webgpu {

class Context;

class VertexBufferResource : public gfx::VertexBufferResource {
public:
    VertexBufferResource(BufferResource&& buffer) noexcept;
    ~VertexBufferResource() noexcept override;

    const BufferResource& getBuffer() const { return buffer; }
    BufferResource& getBuffer() { return buffer; }

    std::size_t getSizeInBytes() const { return buffer.getSize(); }

private:
    BufferResource buffer;
};

} // namespace webgpu
} // namespace mbgl